#include "sql/operator/group_by_physical_operator.h"
#include "sql/expr/tuple.h"
#include "storage/record/record.h"
#include "storage/table/table.h"
#include "common/log/log.h"
#include <limits>

void GroupByPhysicalOperator::add_group_by(const Field &field)
{
  group_by_fields_.push_back(field);
}

void GroupByPhysicalOperator::add_aggregation(AggregationType agg_type, const Table *table,
    const FieldMeta *field_meta, const std::string &alias)
{
  AggregationField agg;
  agg.agg_type = agg_type;
  agg.table = table;
  agg.field_meta = field_meta;
  agg.alias = alias;
  agg_fields_.push_back(agg);
}

RC GroupByPhysicalOperator::open(Trx *trx)
{
  if (children_.empty()) return RC::SUCCESS;
  return children_[0]->open(trx);
}

RC GroupByPhysicalOperator::next()
{
  if (!computed_) {
    computed_ = true;

    if (children_.empty()) {
      return RC::RECORD_EOF;
    }
    PhysicalOperator *child = children_[0].get();

    int n_agg = static_cast<int>(agg_fields_.size());
    int n_gb = static_cast<int>(group_by_fields_.size());

    // Map: string key -> group index
    std::map<std::string, int> group_map;
    // Accumulators per group: sums, max_num, min_num, max_str, min_str, counts
    struct Acc {
      std::vector<double> sum_vals;
      std::vector<double> max_num_vals;
      std::vector<double> min_num_vals;
      std::vector<std::string> max_str_vals;
      std::vector<std::string> min_str_vals;
      std::vector<bool> has_str_max;
      std::vector<bool> has_str_min;
      std::vector<int> counts;
    };
    std::vector<Acc> accs;

    // Collect all rows and group them
    RC rc = RC::SUCCESS;
    while ((rc = child->next()) == RC::SUCCESS) {
      Tuple *tuple = child->current_tuple();

      // Build group key from group_by field values
      std::string group_key;
      std::vector<Value> group_vals;
      for (int i = 0; i < n_gb; i++) {
        const Field &f = group_by_fields_[i];
        Value val;
        TupleCellSpec spec(f.table_name(), f.field_name(), f.field_name());
        RC rc2 = tuple->find_cell(spec, val);
        if (rc2 != RC::SUCCESS) {
          // Try without table name
          TupleCellSpec spec2("", f.field_name(), f.field_name());
          rc2 = tuple->find_cell(spec2, val);
        }
        if (rc2 == RC::SUCCESS) {
          group_vals.push_back(val);
          group_key += val.to_string() + "\x01";
        } else {
          // Default value for missing field
          Value def;
          def.set_int(0);
          group_vals.push_back(def);
          group_key += "0\x01";
        }
      }

      int gi;
      auto it = group_map.find(group_key);
      if (it == group_map.end()) {
        // New group
        gi = static_cast<int>(groups_.size());
        group_map[group_key] = gi;
        groups_.push_back({group_vals, std::vector<Value>()});

        Acc a;
        a.sum_vals.assign(n_agg, 0);
        a.max_num_vals.assign(n_agg, std::numeric_limits<double>::lowest());
        a.min_num_vals.assign(n_agg, std::numeric_limits<double>::max());
        a.max_str_vals.resize(n_agg);
        a.min_str_vals.resize(n_agg);
        a.has_str_max.assign(n_agg, false);
        a.has_str_min.assign(n_agg, false);
        a.counts.assign(n_agg, 0);
        accs.push_back(a);
      } else {
        gi = it->second;
      }

      // Accumulate aggregation values for this group
      Acc &acc = accs[gi];
      for (int i = 0; i < n_agg; i++) {
        AggregationField &af = agg_fields_[i];
        if (af.agg_type == AGG_COUNT && af.field_meta == nullptr) {
          acc.counts[i]++;
          continue;
        }
        if (af.field_meta == nullptr) continue;

        Value val;
        TupleCellSpec spec(af.table->name(), af.field_meta->name(), af.field_meta->name());
        rc = tuple->find_cell(spec, val);
        if (rc != RC::SUCCESS) {
          // Skip if field not found (e.g., from wrong side of join)
          continue;
        }

        AttrType attr_type = val.attr_type();
        if (attr_type == INTS || attr_type == FLOATS) {
          double v = (attr_type == INTS) ? val.get_int() : val.get_float();
          acc.sum_vals[i] += v;
          if (v > acc.max_num_vals[i]) acc.max_num_vals[i] = v;
          if (v < acc.min_num_vals[i]) acc.min_num_vals[i] = v;
          acc.counts[i]++;
        } else if (attr_type == CHARS) {
          std::string s = val.get_string();
          if (af.agg_type == AGG_COUNT) {
            acc.counts[i]++;
          }
          if (af.agg_type == AGG_MAX) {
            if (!acc.has_str_max[i] || s > acc.max_str_vals[i]) {
              acc.max_str_vals[i] = s;
              acc.has_str_max[i] = true;
            }
          }
          if (af.agg_type == AGG_MIN) {
            if (!acc.has_str_min[i] || s < acc.min_str_vals[i]) {
              acc.min_str_vals[i] = s;
              acc.has_str_min[i] = true;
            }
          }
        } else if (attr_type == DATES) {
          double d = val.get_date();
          if (af.agg_type == AGG_COUNT) {
            acc.counts[i]++;
          }
          if (af.agg_type == AGG_MAX) {
            if (d > acc.max_num_vals[i]) acc.max_num_vals[i] = d;
          }
          if (af.agg_type == AGG_MIN) {
            if (d < acc.min_num_vals[i]) acc.min_num_vals[i] = d;
          }
        }
      }
    }

    // Compute final aggregation results for each group
    for (size_t gi = 0; gi < groups_.size(); gi++) {
      Acc &acc = accs[gi];
      std::vector<Value> &agg_vals = groups_[gi].agg_values;

      for (int i = 0; i < n_agg; i++) {
        AggregationField &af = agg_fields_[i];
        Value result;
        switch (af.agg_type) {
          case AGG_COUNT:
            result.set_int(acc.counts[i]);
            break;
          case AGG_SUM:
            if (af.field_meta && af.field_meta->type() == INTS)
              result.set_int((int)acc.sum_vals[i]);
            else
              result.set_float((float)acc.sum_vals[i]);
            break;
          case AGG_AVG:
            if (acc.counts[i] > 0)
              result.set_float((float)(acc.sum_vals[i] / acc.counts[i]));
            else
              result.set_float(0);
            break;
          case AGG_MAX:
            if (af.field_meta && af.field_meta->type() == CHARS) {
              if (acc.has_str_max[i])
                result.set_string(acc.max_str_vals[i].c_str());
              else
                result.set_string("");
            } else if (af.field_meta && af.field_meta->type() == DATES) {
              result.set_date((int)acc.max_num_vals[i]);
            } else if (af.field_meta && af.field_meta->type() == INTS) {
              result.set_int((int)acc.max_num_vals[i]);
            } else {
              result.set_float((float)acc.max_num_vals[i]);
            }
            break;
          case AGG_MIN:
            if (af.field_meta && af.field_meta->type() == CHARS) {
              if (acc.has_str_min[i])
                result.set_string(acc.min_str_vals[i].c_str());
              else
                result.set_string("");
            } else if (af.field_meta && af.field_meta->type() == DATES) {
              result.set_date((int)acc.min_num_vals[i]);
            } else if (af.field_meta && af.field_meta->type() == INTS) {
              result.set_int((int)acc.min_num_vals[i]);
            } else {
              result.set_float((float)acc.min_num_vals[i]);
            }
            break;
          default:
            result.set_int(0);
        }
        agg_vals.push_back(result);
      }
    }
  }

  current_group_index_++;
  if (current_group_index_ >= static_cast<int>(groups_.size())) {
    return RC::RECORD_EOF;
  }

  // Build output tuple: group_by values + aggregation values
  const GroupResult &gr = groups_[current_group_index_];
  std::vector<Value> cells;
  for (const Value &v : gr.group_values) {
    cells.push_back(v);
  }
  for (const Value &v : gr.agg_values) {
    cells.push_back(v);
  }
  result_tuple_.set_cells(cells);

  return RC::SUCCESS;
}

RC GroupByPhysicalOperator::close()
{
  if (!children_.empty()) children_[0]->close();
  return RC::SUCCESS;
}

Tuple *GroupByPhysicalOperator::current_tuple()
{
  return &result_tuple_;
}

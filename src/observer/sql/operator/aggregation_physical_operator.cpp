#include "sql/operator/aggregation_physical_operator.h"
#include "storage/record/record.h"
#include "storage/table/table.h"
#include "common/log/log.h"
#include <limits>

void AggregationPhysicalOperator::add_aggregation(AggregationType agg_type, const Table *table,
    const FieldMeta *field_meta, const std::string &alias)
{
  AggregationField agg;
  agg.agg_type = agg_type;
  agg.table = table;
  agg.field_meta = field_meta;
  agg.alias = alias;
  agg_fields_.push_back(agg);
}

RC AggregationPhysicalOperator::open(Trx *trx)
{
  if (children_.empty()) return RC::SUCCESS;
  return children_[0]->open(trx);
}

RC AggregationPhysicalOperator::next()
{
  if (returned_) return RC::RECORD_EOF;
  if (computed_) {
    returned_ = true;
    return RC::SUCCESS;
  }

  if (children_.empty()) return RC::RECORD_EOF;
  PhysicalOperator *child = children_[0].get();

  int n = agg_fields_.size();
  std::vector<double> sum_vals(n, 0);
  std::vector<double> max_num_vals(n, std::numeric_limits<double>::lowest());
  std::vector<double> min_num_vals(n, std::numeric_limits<double>::max());
  std::vector<std::string> max_str_vals(n);
  std::vector<std::string> min_str_vals(n);
  std::vector<bool> has_str_max(n, false);
  std::vector<bool> has_str_min(n, false);
  std::vector<int> counts(n, 0);

  RC rc = RC::SUCCESS;
  while ((rc = child->next()) == RC::SUCCESS) {
    Tuple *tuple = child->current_tuple();
    for (int i = 0; i < n; i++) {
      AggregationField &af = agg_fields_[i];
      if (af.agg_type == AGG_COUNT && af.field_meta == nullptr) {
        counts[i]++;
        continue;
      }
      if (af.field_meta == nullptr) continue;

      Value val;
      TupleCellSpec spec(af.table->name(), af.field_meta->name(), af.field_meta->name());
      rc = tuple->find_cell(spec, val);
      if (rc != RC::SUCCESS) {
        TupleCellSpec spec2("", af.field_meta->name(), af.field_meta->name());
        rc = tuple->find_cell(spec2, val);
      }
      if (rc != RC::SUCCESS) continue;
      if (val.is_null()) continue;

      AttrType attr_type = val.attr_type();
      if (attr_type == INTS || attr_type == FLOATS) {
        double v = (attr_type == INTS) ? val.get_int() : val.get_float();
        sum_vals[i] += v;
        if (v > max_num_vals[i]) max_num_vals[i] = v;
        if (v < min_num_vals[i]) min_num_vals[i] = v;
        counts[i]++;
      } else if (attr_type == CHARS) {
        std::string s = val.get_string();
        if (af.agg_type == AGG_COUNT) {
          counts[i]++;
        }
        if (af.agg_type == AGG_MAX) {
          if (!has_str_max[i] || s > max_str_vals[i]) {
            max_str_vals[i] = s;
            has_str_max[i] = true;
          }
        }
        if (af.agg_type == AGG_MIN) {
          if (!has_str_min[i] || s < min_str_vals[i]) {
            min_str_vals[i] = s;
            has_str_min[i] = true;
          }
        }
        if (af.agg_type == AGG_MIN || af.agg_type == AGG_MAX || af.agg_type == AGG_SUM) {
          counts[i]++;
        }
      } else if (attr_type == DATES) {
        double d = val.get_date();
        if (af.agg_type == AGG_COUNT) {
          counts[i]++;
        }
        if (af.agg_type == AGG_MAX) {
          if (d > max_num_vals[i]) max_num_vals[i] = d;
          counts[i]++;
        }
        if (af.agg_type == AGG_MIN) {
          if (d < min_num_vals[i]) min_num_vals[i] = d;
          counts[i]++;
        }
      }
    }
  }

  std::vector<Value> results;
  std::vector<std::string> aliases;

  for (int i = 0; i < n; i++) {
    AggregationField &af = agg_fields_[i];
    Value result;
    switch (af.agg_type) {
      case AGG_COUNT:
        result.set_int(counts[i]);
        break;
      case AGG_SUM:
        if (counts[i] == 0) {
          result.set_null(true);
        } else if (af.field_meta && af.field_meta->type() == INTS) {
          result.set_int((int)sum_vals[i]);
        } else {
          result.set_float((float)sum_vals[i]);
        }
        break;
      case AGG_AVG:
        if (counts[i] == 0) {
          result.set_null(true);
        } else {
          result.set_float((float)(sum_vals[i] / counts[i]));
        }
        break;
      case AGG_MAX:
        if (af.field_meta && af.field_meta->type() == CHARS) {
          if (!has_str_max[i])
            result.set_null(true);
          else
            result.set_string(max_str_vals[i].c_str());
        } else if (counts[i] == 0) {
          result.set_null(true);
        } else if (af.field_meta && af.field_meta->type() == DATES) {
          result.set_date((int)max_num_vals[i]);
        } else if (af.field_meta && af.field_meta->type() == INTS) {
          result.set_int((int)max_num_vals[i]);
        } else {
          result.set_float((float)max_num_vals[i]);
        }
        break;
      case AGG_MIN:
        if (af.field_meta && af.field_meta->type() == CHARS) {
          if (!has_str_min[i])
            result.set_null(true);
          else
            result.set_string(min_str_vals[i].c_str());
        } else if (counts[i] == 0) {
          result.set_null(true);
        } else if (af.field_meta && af.field_meta->type() == DATES) {
          result.set_date((int)min_num_vals[i]);
        } else if (af.field_meta && af.field_meta->type() == INTS) {
          result.set_int((int)min_num_vals[i]);
        } else {
          result.set_float((float)min_num_vals[i]);
        }
        break;
      default:
        result.set_int(0);
    }
    results.push_back(result);
    aliases.push_back(af.alias);
  }

  result_tuple_.set_cells(results);
  result_tuple_.set_aliases(aliases);

  computed_ = true;
  returned_ = true;
  return RC::SUCCESS;
}

RC AggregationPhysicalOperator::close()
{
  if (!children_.empty()) children_[0]->close();
  return RC::SUCCESS;
}

Tuple *AggregationPhysicalOperator::current_tuple()
{
  return &result_tuple_;
}
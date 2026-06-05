#include "sql/operator/group_by_physical_operator.h"
#include "sql/expr/tuple.h"
#include "sql/expr/expression.h"
#include "sql/stmt/filter_stmt.h"
#include "storage/record/record.h"
#include "storage/table/table.h"
#include "common/log/log.h"
#include "common/lang/string.h"
#include <limits>
#include <memory>

// Tuple view over one GROUP BY result row for HAVING evaluation.
class PostAggTuple : public Tuple
{
public:
  PostAggTuple(const std::vector<Field> &group_by_fields,
      const std::vector<AggregationField> &agg_fields,
      const std::vector<Value> &group_values, const std::vector<Value> &agg_values)
      : group_by_fields_(group_by_fields),
        agg_fields_(agg_fields),
        group_values_(group_values),
        agg_values_(agg_values)
  {}

  int cell_num() const override
  {
    return static_cast<int>(group_values_.size() + agg_values_.size());
  }

  RC cell_at(int index, Value &cell) const override
  {
    if (index < 0 || index >= cell_num()) {
      return RC::NOTFOUND;
    }
    cell = index < static_cast<int>(group_values_.size()) ? group_values_[index] : agg_values_[index - group_values_.size()];
    return RC::SUCCESS;
  }

  RC find_cell(const TupleCellSpec &spec, Value &cell) const override
  {
    const char *tbl = spec.table_name();
    const char *fld = spec.field_name();
    const char *alias = spec.alias();

    for (size_t i = 0; i < group_by_fields_.size(); i++) {
      const Field &f = group_by_fields_[i];
      if ((fld && strcmp(fld, f.field_name()) == 0) &&
          (common::is_blank(tbl) || strcmp(tbl, f.table_name()) == 0)) {
        cell = group_values_[i];
        return RC::SUCCESS;
      }
      if (alias && strcmp(alias, f.field_name()) == 0) {
        cell = group_values_[i];
        return RC::SUCCESS;
      }
    }

    for (size_t i = 0; i < agg_fields_.size(); i++) {
      const AggregationField &af = agg_fields_[i];
      if (af.field_meta == nullptr) {
        continue;
      }
      if (fld && strcmp(fld, af.field_meta->name()) == 0) {
        cell = agg_values_[i];
        return RC::SUCCESS;
      }
      if (alias && strcmp(alias, af.alias.c_str()) == 0) {
        cell = agg_values_[i];
        return RC::SUCCESS;
      }
      if (alias && af.field_meta && strcmp(alias, af.field_meta->name()) == 0) {
        cell = agg_values_[i];
        return RC::SUCCESS;
      }
    }
    return RC::NOTFOUND;
  }

private:
  const std::vector<Field> &group_by_fields_;
  const std::vector<AggregationField> &agg_fields_;
  const std::vector<Value> &group_values_;
  const std::vector<Value> &agg_values_;
};

static RC eval_filter_unit(const FilterUnit *unit, const Tuple &tuple, bool &result)
{
  if (unit == nullptr) {
    result = true;
    return RC::SUCCESS;
  }
  if (unit->has_custom_expr()) {
    Value v;
    RC rc = unit->custom_expr()->get_value(tuple, v);
    if (rc != RC::SUCCESS) {
      return rc;
    }
    result = v.get_boolean();
    return RC::SUCCESS;
  }
  if (unit->is_expr_based()) {
    Value left_value;
    Value right_value;
    RC rc = unit->left_expr()->get_value(tuple, left_value);
    if (rc != RC::SUCCESS) {
      return rc;
    }
    rc = unit->right_expr()->get_value(tuple, right_value);
    if (rc != RC::SUCCESS) {
      return rc;
    }
    ComparisonExpr cmp(unit->comp(), std::unique_ptr<Expression>(), std::unique_ptr<Expression>());
    return cmp.compare_value(left_value, right_value, result);
  }

  Value left_value;
  Value right_value;
  if (unit->left().is_attr) {
    TupleCellSpec spec(unit->left().field.table_name(), unit->left().field.field_name());
    RC rc = tuple.find_cell(spec, left_value);
    if (rc != RC::SUCCESS) {
      return rc;
    }
  } else {
    left_value = unit->left().value;
  }
  if (unit->right().is_attr) {
    TupleCellSpec spec(unit->right().field.table_name(), unit->right().field.field_name());
    RC rc = tuple.find_cell(spec, right_value);
    if (rc != RC::SUCCESS) {
      return rc;
    }
  } else {
    right_value = unit->right().value;
  }
  ComparisonExpr cmp(unit->comp(), std::unique_ptr<Expression>(), std::unique_ptr<Expression>());
  return cmp.compare_value(left_value, right_value, result);
}

bool GroupByPhysicalOperator::passes_having(const GroupResult &group) const
{
  if (having_filter_ == nullptr || having_filter_->filter_units().empty()) {
    return true;
  }
  PostAggTuple tuple(group_by_fields_, agg_fields_, group.group_values, group.agg_values);
  for (const FilterUnit *unit : having_filter_->filter_units()) {
    bool ok = false;
    if (eval_filter_unit(unit, tuple, ok) != RC::SUCCESS) {
      return false;
    }
    if (!ok) {
      return false;
    }
  }
  return true;
}

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
          TupleCellSpec spec2("", af.field_meta->name(), af.field_meta->name());
          rc = tuple->find_cell(spec2, val);
        }
        if (rc != RC::SUCCESS) {
          continue;
        }
        if (val.is_null()) {
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
          if (af.agg_type == AGG_MIN || af.agg_type == AGG_MAX || af.agg_type == AGG_SUM) {
            acc.counts[i]++;
          }
        } else if (attr_type == DATES) {
          double d = val.get_date();
          if (af.agg_type == AGG_COUNT) {
            acc.counts[i]++;
          }
          if (af.agg_type == AGG_MAX) {
            if (d > acc.max_num_vals[i]) acc.max_num_vals[i] = d;
            acc.counts[i]++;
          }
          if (af.agg_type == AGG_MIN) {
            if (d < acc.min_num_vals[i]) acc.min_num_vals[i] = d;
            acc.counts[i]++;
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
            if (acc.counts[i] == 0) {
              result.set_null(true);
            } else if (af.field_meta && af.field_meta->type() == INTS) {
              result.set_int((int)acc.sum_vals[i]);
            } else {
              result.set_float((float)acc.sum_vals[i]);
            }
            break;
          case AGG_AVG:
            if (acc.counts[i] == 0) {
              result.set_null(true);
            } else {
              result.set_float((float)(acc.sum_vals[i] / acc.counts[i]));
            }
            break;
          case AGG_MAX:
            if (af.field_meta && af.field_meta->type() == CHARS) {
              if (!acc.has_str_max[i])
                result.set_null(true);
              else
                result.set_string(acc.max_str_vals[i].c_str());
            } else if (acc.counts[i] == 0) {
              result.set_null(true);
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
              if (!acc.has_str_min[i])
                result.set_null(true);
              else
                result.set_string(acc.min_str_vals[i].c_str());
            } else if (acc.counts[i] == 0) {
              result.set_null(true);
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

    if (having_filter_ != nullptr) {
      std::vector<GroupResult> filtered;
      filtered.reserve(groups_.size());
      for (const GroupResult &gr : groups_) {
        if (passes_having(gr)) {
          filtered.push_back(gr);
        }
      }
      groups_.swap(filtered);
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

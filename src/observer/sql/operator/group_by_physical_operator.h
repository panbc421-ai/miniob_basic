#pragma once
#include <vector>
#include <string>
#include <map>
#include "sql/operator/physical_operator.h"
#include "sql/parser/parse_defs.h"
#include "sql/operator/aggregation_physical_operator.h"
#include "sql/expr/tuple.h"
#include "storage/field/field.h"

/**
 * @brief GROUP BY 物理算子
 * @ingroup PhysicalOperator
 * @details 接收子算子产出的所有行，按 GROUP BY 字段分组后计算聚合
 */
class GroupByPhysicalOperator : public PhysicalOperator
{
public:
  GroupByPhysicalOperator() = default;
  virtual ~GroupByPhysicalOperator() = default;

  PhysicalOperatorType type() const override
  {
    return PhysicalOperatorType::AGGREGATION;
  }

  RC open(Trx *trx) override;
  RC next() override;
  RC close() override;
  Tuple *current_tuple() override;

  void add_group_by(const Field &field);
  void add_aggregation(AggregationType agg_type, const Table *table,
                       const FieldMeta *field_meta, const std::string &alias);

private:
  std::vector<Field> group_by_fields_;
  std::vector<AggregationField> agg_fields_;

  // result storage
  struct GroupResult {
    std::vector<Value> group_values;
    std::vector<Value> agg_values;
  };
  std::vector<GroupResult> groups_;
  int current_group_index_ = -1;
  bool computed_ = false;

  ValueListTuple result_tuple_;
};

#pragma once
#include <vector>
#include <string>
#include "sql/operator/physical_operator.h"
#include "sql/parser/parse_defs.h"
#include "sql/expr/tuple.h"
#include "storage/field/field_meta.h"

struct AggregationField {
  AggregationType agg_type;
  const Table *table = nullptr;
  const FieldMeta *field_meta = nullptr;
  std::string alias;
};

class AggregationTuple : public Tuple
{
public:
  void set_cells(const std::vector<Value> &cells) { cells_ = cells; }
  void set_aliases(const std::vector<std::string> &aliases) { aliases_ = aliases; }

  int cell_num() const override { return cells_.size(); }

  RC cell_at(int index, Value &cell) const override {
    if (index < 0 || index >= (int)cells_.size()) return RC::NOTFOUND;
    cell = cells_[index];
    return RC::SUCCESS;
  }

  RC find_cell(const TupleCellSpec &spec, Value &cell) const override {
    for (int i = 0; i < (int)aliases_.size(); i++) {
      if (aliases_[i] == spec.alias()) {
        cell = cells_[i];
        return RC::SUCCESS;
      }
    }
    return RC::NOTFOUND;
  }

private:
  std::vector<Value> cells_;
  std::vector<std::string> aliases_;
};

class AggregationPhysicalOperator : public PhysicalOperator
{
public:
  AggregationPhysicalOperator() = default;
  virtual ~AggregationPhysicalOperator() = default;

  PhysicalOperatorType type() const override { return PhysicalOperatorType::AGGREGATION; }

  RC open(Trx *trx) override;
  RC next() override;
  RC close() override;
  Tuple *current_tuple() override;

  void add_aggregation(AggregationType agg_type, const Table *table,
                       const FieldMeta *field_meta, const std::string &alias);

private:
  std::vector<AggregationField> agg_fields_;
  AggregationTuple result_tuple_;
  bool computed_ = false;
  bool returned_ = false;
};
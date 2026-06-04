#pragma once
#include <vector>
#include <string>
#include "sql/operator/physical_operator.h"
#include "sql/parser/parse_defs.h"

class SortTuple : public Tuple
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

class SortPhysicalOperator : public PhysicalOperator
{
public:
  SortPhysicalOperator() = default;
  virtual ~SortPhysicalOperator() = default;

  PhysicalOperatorType type() const override { return PhysicalOperatorType::SORT; }

  RC open(Trx *trx) override;
  RC next() override;
  RC close() override;
  Tuple *current_tuple() override;

  void add_order_by(const OrderByNode &node) { order_by_.push_back(node); }

private:
  struct SortEntry {
    std::vector<Value> sort_keys;
    std::vector<Value> cells;
    std::vector<std::string> aliases;
  };

  std::vector<OrderByNode> order_by_;
  std::vector<SortEntry> entries_;
  int current_index_ = -1;
  bool sorted_ = false;
  SortTuple result_tuple_;
};

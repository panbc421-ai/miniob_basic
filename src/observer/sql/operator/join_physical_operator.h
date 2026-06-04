/* Copyright (c) 2021 OceanBase and/or its affiliates. All rights reserved.
miniob is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
         http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

//
// Created by WangYunlai on 2021/6/10.
//

#pragma once

#include "sql/parser/parse.h"
#include "sql/operator/physical_operator.h"

class Table;

/**
 * @brief 缓存的右表行作为 Tuple，支持 find_cell 按字段名查找
 * @ingroup Tuple
 */
class CachedRowTuple : public Tuple
{
public:
  CachedRowTuple() = default;
  virtual ~CachedRowTuple() = default;

  void set_row(const std::string *table_name,
               const std::vector<std::string> *field_names,
               const std::vector<Value> *values)
  {
    table_name_ = table_name;
    field_names_ = field_names;
    values_ = values;
  }

  int cell_num() const override
  {
    if (values_ == nullptr) return 0;
    return static_cast<int>(values_->size());
  }

  RC cell_at(int index, Value &cell) const override
  {
    if (values_ == nullptr || index < 0 || index >= static_cast<int>(values_->size()))
      return RC::NOTFOUND;
    cell = (*values_)[index];
    return RC::SUCCESS;
  }

  RC find_cell(const TupleCellSpec &spec, Value &cell) const override
  {
    if (field_names_ == nullptr) return RC::NOTFOUND;
    const char *table = spec.table_name();
    const char *field = spec.field_name();
    // 表名非空时必须匹配表名
    if (table != nullptr && table[0] != '\0'
        && table_name_ != nullptr
        && 0 != strcmp(table, table_name_->c_str())) {
      return RC::NOTFOUND;
    }
    for (size_t i = 0; i < field_names_->size(); i++) {
      if (0 == strcmp(field, (*field_names_)[i].c_str())) {
        cell = (*values_)[i];
        return RC::SUCCESS;
      }
    }
    return RC::NOTFOUND;
  }

private:
  const std::string              *table_name_ = nullptr;
  const std::vector<std::string> *field_names_ = nullptr;
  const std::vector<Value>       *values_ = nullptr;
};

/**
 * @brief 嵌套循环连接算子，对右子树为表扫描时做全量缓存优化
 * @ingroup PhysicalOperator
 */
class NestedLoopJoinPhysicalOperator : public PhysicalOperator
{
public:
  NestedLoopJoinPhysicalOperator();
  virtual ~NestedLoopJoinPhysicalOperator() = default;

  PhysicalOperatorType type() const override
  {
    return PhysicalOperatorType::NESTED_LOOP_JOIN;
  }

  RC open(Trx *trx) override;
  RC next() override;
  RC close() override;
  Tuple *current_tuple() override;

private:
  RC left_next();
  RC right_next();
  bool try_cache_right(Trx *trx);

private:
  Trx *trx_ = nullptr;

  PhysicalOperator *left_ = nullptr;
  PhysicalOperator *right_ = nullptr;
  Tuple *left_tuple_ = nullptr;
  Tuple *right_tuple_ = nullptr;
  JoinedTuple joined_tuple_;
  bool round_done_ = true;
  bool right_closed_ = true;

  // 缓存优化：当右子树为 TableScan 时，全量缓存到内存
  bool right_cached_ = false;
  std::string right_table_name_;
  std::vector<std::string> right_field_names_;
  std::vector<std::vector<Value>> right_rows_;
  CachedRowTuple cached_tuple_;
  size_t cached_idx_ = 0;
};

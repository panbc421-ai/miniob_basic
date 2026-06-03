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
// Created by Wangyunlai on 2022/5/22.
//

#pragma once

#include <vector>
#include <unordered_map>
#include "sql/parser/parse_defs.h"
#include "sql/stmt/stmt.h"
#include "sql/expr/expression.h"

class Db;
class Table;
class FieldMeta;

struct FilterObj 
{
  bool is_attr;
  Field field;
  Value value;

  void init_attr(const Field &field)
  {
    is_attr = true;
    this->field = field;
  }

  void init_value(const Value &value)
  {
    is_attr = false;
    this->value = value;
  }
};

class FilterUnit 
{
public:
  FilterUnit() = default;
  ~FilterUnit()
  {}

  void set_comp(CompOp comp)
  {
    comp_ = comp;
  }

  CompOp comp() const
  {
    return comp_;
  }

  void set_left(const FilterObj &obj)
  {
    left_ = obj;
  }
  void set_right(const FilterObj &obj)
  {
    right_ = obj;
  }

  const FilterObj &left() const
  {
    return left_;
  }
  const FilterObj &right() const
  {
    return right_;
  }

  bool is_expr_based() const { return left_expr_ != nullptr || right_expr_ != nullptr; }
  Expression *left_expr() const { return left_expr_; }
  Expression *right_expr() const { return right_expr_; }
  void set_left_expr(Expression *e) { left_expr_ = e; }
  void set_right_expr(Expression *e) { right_expr_ = e; }

  bool has_custom_expr() const { return custom_expr_ != nullptr; }
  Expression *custom_expr() const { return custom_expr_; }
  void set_custom_expr(Expression *e) { custom_expr_ = e; }

private:
  CompOp comp_ = NO_OP;
  FilterObj left_;
  FilterObj right_;
  Expression *left_expr_ = nullptr;
  Expression *right_expr_ = nullptr;
  Expression *custom_expr_ = nullptr;
};

/**
 * @brief Filter/谓词/过滤语句
 * @ingroup Statement
 */
class FilterStmt 
{
public:
  FilterStmt() = default;
  virtual ~FilterStmt();

public:
  const std::vector<FilterUnit *> &filter_units() const
  {
    return filter_units_;
  }

public:
  static RC create(Db *db, Table *default_table, std::unordered_map<std::string, Table *> *tables,
      const ConditionSqlNode *conditions, int condition_num, FilterStmt *&stmt,
      std::unordered_map<std::string, Table *> *outer_table_map = nullptr);

  static RC create_filter_unit(Db *db, Table *default_table, std::unordered_map<std::string, Table *> *tables,
      const ConditionSqlNode &condition, FilterUnit *&filter_unit,
      std::unordered_map<std::string, Table *> *outer_table_map = nullptr);

private:
  std::vector<FilterUnit *> filter_units_;  // 默认当前都是AND关系
};

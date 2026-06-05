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
#include <string>
#include <vector>
#include "common/rc.h"
#include "sql/stmt/stmt.h"
#include "sql/stmt/filter_stmt.h"
#include "sql/parser/value.h"

class Table;
class FieldMeta;
struct SelectSqlNode;

/**
 * @brief 一个待更新字段的解析结果
 */
struct UpdateUnit
{
  const FieldMeta     *field_meta = nullptr;   ///< 目标字段
  Value                value;                   ///< 常量值（is_subquery=false 时）
  bool                 is_subquery = false;     ///< 是否来自子查询
  const SelectSqlNode *subquery = nullptr;      ///< 子查询（非拥有，随 ParsedSqlNode 存活）
};

class UpdateStmt : public Stmt 
{
public:
  UpdateStmt() = default;
  ~UpdateStmt();

public:
  static RC create(Db *db, const UpdateSqlNode &update_sql, Stmt *&stmt);

public:
  StmtType type() const override { return StmtType::UPDATE; }
  Table *table() const { return table_; }
  const std::vector<UpdateUnit> &units() const { return units_; }
  FilterStmt *filter_stmt() const { return filter_stmt_; }

private:
  Table *table_ = nullptr;
  std::vector<UpdateUnit> units_;
  FilterStmt *filter_stmt_ = nullptr;
};
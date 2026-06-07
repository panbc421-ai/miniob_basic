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
#include "sql/stmt/update_stmt.h"
#include "sql/stmt/filter_stmt.h"
#include "storage/db/db.h"
#include "storage/table/table.h"
#include "sql/parser/condition_clone.h"
#include "common/log/log.h"
#include <unordered_map>

UpdateStmt::~UpdateStmt()
{
  if (filter_stmt_ != nullptr) {
    delete filter_stmt_;
    filter_stmt_ = nullptr;
  }
}

RC UpdateStmt::create(Db *db, const UpdateSqlNode &update, Stmt *&stmt)
{
  const char *table_name = update.relation_name.c_str();
  Table *table = db->find_table(table_name);
  if (table == nullptr) {
    LOG_WARN("no such table. db=%s, table_name=%s", db->name(), table_name);
    return RC::SCHEMA_TABLE_NOT_EXIST;
  }
  if (db->is_readonly_view(table_name)) {
    LOG_WARN("cannot update readonly view. view=%s", table_name);
    return RC::INVALID_ARGUMENT;
  }

  if (update.assignments.empty()) {
    LOG_WARN("update has no assignment. table=%s", table_name);
    return RC::INVALID_ARGUMENT;
  }

  // Resolve each SET assignment to its target field.
  std::vector<UpdateUnit> units;
  for (const UpdateAssignmentNode &assign : update.assignments) {
    const FieldMeta *field_meta = table->table_meta().field(assign.attribute_name.c_str());
    if (field_meta == nullptr) {
      LOG_WARN("no such field. table=%s, field=%s", table_name, assign.attribute_name.c_str());
      return RC::SCHEMA_FIELD_NOT_EXIST;
    }
    UpdateUnit unit;
    unit.field_meta = field_meta;
    unit.is_subquery = assign.is_subquery;
    unit.subquery = assign.subquery;
    unit.value = assign.value;
    units.push_back(unit);
  }

  FilterStmt *filter_stmt = nullptr;
  std::unordered_map<std::string, Table *> table_map;
  table_map[table_name] = table;
  const char *base_table_name = db->view_base_table(table_name);
  if (base_table_name != nullptr) {
    table_map[base_table_name] = table;
  }

  RC rc = RC::SUCCESS;
  std::vector<ConditionSqlNode> combined_conditions;
  const std::vector<ConditionSqlNode> *view_conditions = db->view_conditions(table_name);
  bool has_view_conditions = view_conditions != nullptr && !view_conditions->empty();
  if (has_view_conditions) {
    rc = append_cloned_conditions(*view_conditions, combined_conditions);
    if (rc != RC::SUCCESS) {
      return rc;
    }
    rc = append_cloned_conditions(update.conditions, combined_conditions);
    if (rc != RC::SUCCESS) {
      return rc;
    }
  }
  const ConditionSqlNode *filter_conditions =
      has_view_conditions ? combined_conditions.data() : update.conditions.data();
  const int filter_condition_num = has_view_conditions
      ? static_cast<int>(combined_conditions.size())
      : static_cast<int>(update.conditions.size());

  rc = FilterStmt::create(db, table, &table_map,
      filter_conditions, filter_condition_num, filter_stmt);
  if (rc != RC::SUCCESS) {
    LOG_WARN("failed to create filter statement. rc=%d:%s", rc, strrc(rc));
    return rc;
  }

  UpdateStmt *update_stmt = new UpdateStmt();
  update_stmt->table_ = table;
  update_stmt->units_.swap(units);
  update_stmt->filter_stmt_ = filter_stmt;
  stmt = update_stmt;
  return RC::SUCCESS;
}

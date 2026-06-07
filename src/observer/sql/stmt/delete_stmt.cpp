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

#include "common/log/log.h"
#include "sql/stmt/delete_stmt.h"
#include "sql/stmt/filter_stmt.h"
#include "sql/parser/condition_clone.h"
#include "storage/db/db.h"
#include "storage/table/table.h"

DeleteStmt::DeleteStmt(Table *table, FilterStmt *filter_stmt) : table_(table), filter_stmt_(filter_stmt)
{}

DeleteStmt::~DeleteStmt()
{
  if (nullptr != filter_stmt_) {
    delete filter_stmt_;
    filter_stmt_ = nullptr;
  }
}

RC DeleteStmt::create(Db *db, const DeleteSqlNode &delete_sql, Stmt *&stmt)
{
  const char *table_name = delete_sql.relation_name.c_str();
  if (nullptr == db || nullptr == table_name) {
    LOG_WARN("invalid argument. db=%p, table_name=%p", db, table_name);
    return RC::INVALID_ARGUMENT;
  }

  // check whether the table exists
  Table *table = db->find_table(table_name);
  if (nullptr == table) {
    LOG_WARN("no such table. db=%s, table_name=%s", db->name(), table_name);
    return RC::SCHEMA_TABLE_NOT_EXIST;
  }

  std::unordered_map<std::string, Table *> table_map;
  table_map.insert(std::pair<std::string, Table *>(std::string(table_name), table));
  const char *base_table_name = db->view_base_table(table_name);
  if (base_table_name != nullptr) {
    table_map.insert(std::pair<std::string, Table *>(std::string(base_table_name), table));
  }

  std::vector<ConditionSqlNode> combined_conditions;
  const std::vector<ConditionSqlNode> *view_conditions = db->view_conditions(table_name);
  bool has_view_conditions = view_conditions != nullptr && !view_conditions->empty();
  if (has_view_conditions) {
    RC clone_rc = append_cloned_conditions(*view_conditions, combined_conditions);
    if (clone_rc != RC::SUCCESS) {
      return clone_rc;
    }
    clone_rc = append_cloned_conditions(delete_sql.conditions, combined_conditions);
    if (clone_rc != RC::SUCCESS) {
      return clone_rc;
    }
  }
  const ConditionSqlNode *filter_conditions =
      has_view_conditions ? combined_conditions.data() : delete_sql.conditions.data();
  const int filter_condition_num = has_view_conditions
      ? static_cast<int>(combined_conditions.size())
      : static_cast<int>(delete_sql.conditions.size());

  FilterStmt *filter_stmt = nullptr;
  RC rc = FilterStmt::create(
      db, table, &table_map, filter_conditions, filter_condition_num, filter_stmt);
  if (rc != RC::SUCCESS) {
    LOG_WARN("failed to create filter statement. rc=%d:%s", rc, strrc(rc));
    return rc;
  }

  stmt = new DeleteStmt(table, filter_stmt);
  return rc;
}

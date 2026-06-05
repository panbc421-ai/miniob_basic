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
// Created by Wangyunlai on 2023/6/13.
//

#include "sql/executor/create_table_executor.h"

#include "session/session.h"
#include "common/log/log.h"
#include "storage/table/table.h"
#include "sql/stmt/create_table_stmt.h"
#include "sql/stmt/select_stmt.h"
#include "sql/optimizer/logical_plan_generator.h"
#include "sql/optimizer/physical_plan_generator.h"
#include "sql/expr/expression.h"
#include "sql/expr/tuple.h"
#include "event/sql_event.h"
#include "event/session_event.h"
#include "storage/db/db.h"
#include "storage/trx/trx.h"
#include <memory>
#include <vector>

static size_t default_attr_len(AttrType type)
{
  switch (type) {
    case INTS:
    case FLOATS:
    case DATES:
    case BOOLEANS: return 4;
    case TEXTS: return 4096;
    case CHARS: return 4096;
    default: return 4;
  }
}

static AttrInfoSqlNode attr_from_field(const Field &field)
{
  AttrInfoSqlNode attr;
  attr.type = field.meta()->type();
  attr.name = field.field_name();
  attr.length = field.meta()->len();
  attr.nullable = field.meta()->nullable();
  return attr;
}

static AttrInfoSqlNode attr_from_expr(const SelectExprNode &select_expr, int index)
{
  AttrInfoSqlNode attr;
  Expression *expr = select_expr.expr;
  attr.type = expr != nullptr ? expr->value_type() : CHARS;
  attr.name = !select_expr.alias.empty() ? select_expr.alias :
      (expr != nullptr && !expr->name().empty() ? expr->name() : "col" + std::to_string(index + 1));
  attr.length = default_attr_len(attr.type);
  attr.nullable = true;

  if (expr != nullptr && expr->type() == ExprType::FIELD) {
    FieldExpr *field_expr = static_cast<FieldExpr *>(expr);
    attr.length = field_expr->field().meta()->len();
    attr.nullable = field_expr->field().meta()->nullable();
  }
  return attr;
}

static RC infer_select_attrs(SelectStmt *select_stmt, std::vector<AttrInfoSqlNode> &attrs)
{
  if (!select_stmt->select_exprs().empty()) {
    const std::vector<SelectExprNode> &select_exprs = select_stmt->select_exprs();
    for (size_t i = 0; i < select_exprs.size(); i++) {
      attrs.push_back(attr_from_expr(select_exprs[i], static_cast<int>(i)));
    }
    return RC::SUCCESS;
  }

  for (const Field &field : select_stmt->query_fields()) {
    attrs.push_back(attr_from_field(field));
  }
  return attrs.empty() ? RC::INVALID_ARGUMENT : RC::SUCCESS;
}

static RC materialize_select_into_table(Db *db, Trx *trx, const char *table_name, const SelectSqlNode &select_sql)
{
  Stmt *inner_stmt = nullptr;
  RC rc = SelectStmt::create(db, select_sql, inner_stmt);
  if (rc != RC::SUCCESS) {
    return rc;
  }
  std::unique_ptr<Stmt> stmt_guard(inner_stmt);
  if (inner_stmt == nullptr || inner_stmt->type() != StmtType::SELECT) {
    return RC::INVALID_ARGUMENT;
  }

  SelectStmt *select_stmt = static_cast<SelectStmt *>(inner_stmt);
  std::vector<AttrInfoSqlNode> attrs;
  rc = infer_select_attrs(select_stmt, attrs);
  if (rc != RC::SUCCESS) {
    return rc;
  }

  rc = db->create_table(table_name, static_cast<int>(attrs.size()), attrs.data());
  if (rc != RC::SUCCESS) {
    return rc;
  }

  Table *target_table = db->find_table(table_name);
  if (target_table == nullptr) {
    return RC::SCHEMA_TABLE_NOT_EXIST;
  }

  LogicalPlanGenerator logical_generator;
  std::unique_ptr<LogicalOperator> logical_oper;
  rc = logical_generator.create(select_stmt, logical_oper);
  if (rc != RC::SUCCESS) {
    return rc;
  }

  PhysicalPlanGenerator physical_generator;
  std::unique_ptr<PhysicalOperator> physical_oper;
  rc = physical_generator.create(*logical_oper, physical_oper);
  if (rc != RC::SUCCESS) {
    return rc;
  }

  rc = physical_oper->open(trx);
  if (rc != RC::SUCCESS) {
    return rc;
  }

  const int value_num = static_cast<int>(attrs.size());
  while ((rc = physical_oper->next()) == RC::SUCCESS) {
    Tuple *tuple = physical_oper->current_tuple();
    if (tuple == nullptr || tuple->cell_num() != value_num) {
      physical_oper->close();
      return RC::INVALID_ARGUMENT;
    }

    std::vector<Value> values(value_num);
    for (int i = 0; i < value_num; i++) {
      RC cell_rc = tuple->cell_at(i, values[i]);
      if (cell_rc != RC::SUCCESS) {
        physical_oper->close();
        return cell_rc;
      }
    }

    Record record;
    rc = target_table->make_record(value_num, values.data(), record);
    if (rc != RC::SUCCESS) {
      physical_oper->close();
      return rc;
    }
    rc = trx->insert_record(target_table, record);
    if (rc != RC::SUCCESS) {
      physical_oper->close();
      return rc;
    }
  }
  physical_oper->close();

  return rc == RC::RECORD_EOF ? RC::SUCCESS : rc;
}

RC CreateTableExecutor::execute(SQLStageEvent *sql_event)
{
  Stmt *stmt = sql_event->stmt();
  Session *session = sql_event->session_event()->session();
  ASSERT(stmt->type() == StmtType::CREATE_TABLE, 
         "create table executor can not run this command: %d", static_cast<int>(stmt->type()));

  CreateTableStmt *create_table_stmt = static_cast<CreateTableStmt *>(stmt);

  if (create_table_stmt->has_select()) {
    return materialize_select_into_table(session->get_current_db(),
        session->current_trx(),
        create_table_stmt->table_name().c_str(),
        *create_table_stmt->select_sql());
  }

  const int attribute_count = static_cast<int>(create_table_stmt->attr_infos().size());

  const char *table_name = create_table_stmt->table_name().c_str();
  RC rc = session->get_current_db()->create_table(table_name, attribute_count, create_table_stmt->attr_infos().data());

  return rc;
}

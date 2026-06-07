#include "sql/executor/create_view_executor.h"

#include "event/session_event.h"
#include "event/sql_event.h"
#include "session/session.h"
#include "sql/executor/materialize_select.h"
#include "sql/expr/expression.h"
#include "sql/expr/tuple.h"
#include "sql/parser/condition_clone.h"
#include "sql/stmt/create_view_stmt.h"
#include "storage/record/record_manager.h"
#include "storage/db/db.h"
#include "storage/table/table.h"
#include "storage/trx/trx.h"

#include <memory>
#include <unordered_map>

static int default_attr_length(AttrType type)
{
  switch (type) {
    case INTS:
    case FLOATS:
    case DATES:
    case BOOLEANS:
      return 4;
    case TEXTS:
      return 4096;
    case CHARS:
      return 256;
    default:
      return 4;
  }
}

static RC finish_create_view_trx(Trx *trx, bool auto_commit, RC rc)
{
  if (trx == nullptr || !auto_commit) {
    return rc;
  }
  if (rc == RC::SUCCESS) {
    return trx->commit();
  }
  RC rollback_rc = trx->rollback();
  if (rollback_rc != RC::SUCCESS) {
    LOG_PANIC("rollback create view transaction failed. rc=%s", strrc(rollback_rc));
  }
  return rc;
}

static bool is_simple_expression_view(const SelectSqlNode &select_sql)
{
  if (select_sql.relations.size() != 1 || select_sql.expressions.empty() ||
      !select_sql.attributes.empty() || !select_sql.group_by.empty() ||
      !select_sql.having.empty() || !select_sql.order_by.empty() ||
      !select_sql.conditions.empty()) {
    return false;
  }

  for (const SelectExprNode &expr : select_sql.expressions) {
    if (expr.is_star || expr.agg_type != AGG_NONE || expr.expr == nullptr) {
      return false;
    }
  }
  return true;
}

static RC materialize_simple_expression_view(Db *db, Trx *trx, bool auto_commit,
    const char *view_name, const SelectSqlNode &select_sql,
    const std::vector<std::string> *specified_column_names)
{
  if (db == nullptr || trx == nullptr || view_name == nullptr || view_name[0] == '\0') {
    return RC::INVALID_ARGUMENT;
  }

  Table *base_table = db->find_table(select_sql.relations.front().c_str());
  if (base_table == nullptr) {
    return RC::SCHEMA_TABLE_NOT_EXIST;
  }

  RC rc = trx->start_if_need();
  if (rc != RC::SUCCESS) {
    return rc;
  }

  std::unordered_map<std::string, Table *> table_map;
  table_map[select_sql.relations.front()] = base_table;
  if (!select_sql.aliases.empty() && !select_sql.aliases.front().empty()) {
    table_map[select_sql.aliases.front()] = base_table;
  }

  std::vector<std::unique_ptr<Expression>> output_exprs;
  std::vector<AttrInfoSqlNode> attr_infos;
  output_exprs.reserve(select_sql.expressions.size());
  attr_infos.reserve(select_sql.expressions.size());

  for (size_t i = 0; i < select_sql.expressions.size(); i++) {
    const SelectExprNode &select_expr = select_sql.expressions[i];
    std::unique_ptr<Expression> expr = clone_sql_expression(select_expr.expr);
    if (expr == nullptr) {
      return finish_create_view_trx(trx, auto_commit, RC::INVALID_ARGUMENT);
    }

    rc = resolve_expression(expr, base_table, &table_map);
    if (rc != RC::SUCCESS) {
      return finish_create_view_trx(trx, auto_commit, rc);
    }

    std::string name = select_expr.alias.empty() ? expr->name() : select_expr.alias;
    if (specified_column_names != nullptr && !specified_column_names->empty()) {
      if (specified_column_names->size() != select_sql.expressions.size()) {
        return finish_create_view_trx(trx, auto_commit, RC::INVALID_ARGUMENT);
      }
      name = (*specified_column_names)[i];
    }

    AttrInfoSqlNode attr;
    attr.name = name.empty() ? ("col" + std::to_string(i + 1)) : name;
    attr.nullable = true;
    if (expr->type() == ExprType::FIELD) {
      FieldExpr *field_expr = static_cast<FieldExpr *>(expr.get());
      const FieldMeta *field_meta = field_expr->field().meta();
      attr.type = field_meta->type();
      attr.length = field_meta->len();
      attr.nullable = field_meta->nullable();
    } else {
      attr.type = expr->value_type();
      if (attr.type == UNDEFINED || attr.type == NULLS) {
        attr.type = CHARS;
      }
      attr.length = default_attr_length(attr.type);
    }
    attr_infos.emplace_back(attr);
    output_exprs.emplace_back(std::move(expr));
  }

  rc = db->create_table(view_name, static_cast<int>(attr_infos.size()), attr_infos.data());
  if (rc != RC::SUCCESS) {
    return finish_create_view_trx(trx, auto_commit, rc);
  }

  Table *target_table = db->find_table(view_name);
  if (target_table == nullptr) {
    return finish_create_view_trx(trx, auto_commit, RC::SCHEMA_TABLE_NOT_EXIST);
  }

  RecordFileScanner scanner;
  rc = base_table->get_record_scanner(scanner, trx, true);
  if (rc != RC::SUCCESS) {
    return finish_create_view_trx(trx, auto_commit, rc);
  }

  RowTuple tuple;
  tuple.set_schema(base_table, base_table->table_meta().field_metas());
  Record record;
  while (scanner.has_next()) {
    rc = scanner.next(record);
    if (rc != RC::SUCCESS) {
      break;
    }
    tuple.set_record(&record);

    std::vector<Value> values(output_exprs.size());
    for (size_t i = 0; i < output_exprs.size(); i++) {
      rc = output_exprs[i]->get_value(tuple, values[i]);
      if (rc != RC::SUCCESS) {
        break;
      }
    }
    if (rc != RC::SUCCESS) {
      break;
    }

    Record view_record;
    rc = target_table->make_record(static_cast<int>(values.size()), values.data(), view_record);
    if (rc != RC::SUCCESS) {
      break;
    }
    rc = target_table->insert_record(view_record);
    if (rc != RC::SUCCESS) {
      break;
    }
  }
  scanner.close_scan();

  if (rc == RC::RECORD_EOF) {
    rc = RC::SUCCESS;
  }
  return finish_create_view_trx(trx, auto_commit, rc);
}

static bool is_aliasable_star_view(const SelectSqlNode &select_sql)
{
  if (select_sql.relations.size() != 1 || !select_sql.expressions.empty() ||
      !select_sql.group_by.empty() || !select_sql.having.empty() || !select_sql.order_by.empty() ||
      select_sql.attributes.size() != 1) {
    return false;
  }

  const RelAttrSqlNode &attr = select_sql.attributes.front();
  return attr.aggregation_type == AGG_NONE && attr.relation_name.empty() && attr.attribute_name == "*";
}

static bool is_aliasable_column_view(const SelectSqlNode &select_sql)
{
  if (select_sql.relations.size() != 1 || select_sql.expressions.empty() ||
      !select_sql.attributes.empty() || !select_sql.group_by.empty() ||
      !select_sql.having.empty() || !select_sql.order_by.empty()) {
    return false;
  }

  for (const SelectExprNode &expr : select_sql.expressions) {
    if (expr.expr == nullptr || expr.expr->type() != ExprType::UNBOUND_FIELD) {
      return false;
    }
  }
  return true;
}

static std::vector<std::string> selected_base_columns(const SelectSqlNode &select_sql)
{
  std::vector<std::string> columns;
  columns.reserve(select_sql.expressions.size());
  for (const SelectExprNode &expr : select_sql.expressions) {
    auto *field_expr = static_cast<UnboundFieldExpr *>(expr.expr);
    columns.push_back(field_expr->field_name());
  }
  return columns;
}

RC CreateViewExecutor::execute(SQLStageEvent *sql_event)
{
  CreateViewStmt *stmt = static_cast<CreateViewStmt *>(sql_event->stmt());
  Session *session = sql_event->session_event()->session();
  const std::vector<ConditionSqlNode> *conditions =
      stmt->select_sql().conditions.empty() ? nullptr : &stmt->select_sql().conditions;
  if (stmt->column_names().empty() && is_aliasable_star_view(stmt->select_sql())) {
    return session->get_current_db()->create_view_alias(
        stmt->view_name().c_str(), stmt->select_sql().relations.front().c_str(),
        std::vector<std::string>(), conditions);
  }
  if (!stmt->column_names().empty() && is_aliasable_column_view(stmt->select_sql())) {
    return session->get_current_db()->create_view_alias(
        stmt->view_name().c_str(), stmt->select_sql().relations.front().c_str(),
        selected_base_columns(stmt->select_sql()), conditions);
  }

  const bool auto_commit = !session->is_trx_multi_operation_mode();
  if (is_simple_expression_view(stmt->select_sql())) {
    return materialize_simple_expression_view(session->get_current_db(),
        session->current_trx(), auto_commit,
        stmt->view_name().c_str(), stmt->select_sql(),
        stmt->column_names().empty() ? nullptr : &stmt->column_names());
  }

  return materialize_select_as_table(session->get_current_db(),
      session->current_trx(), auto_commit,
      stmt->view_name().c_str(), stmt->select_sql(),
      nullptr, stmt->column_names().empty() ? nullptr : &stmt->column_names(),
      true);
}

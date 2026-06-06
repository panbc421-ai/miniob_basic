#include "sql/executor/create_view_executor.h"

#include "event/session_event.h"
#include "event/sql_event.h"
#include "session/session.h"
#include "sql/executor/materialize_select.h"
#include "sql/expr/expression.h"
#include "sql/stmt/create_view_stmt.h"
#include "storage/db/db.h"

static bool is_simple_star_view(const SelectSqlNode &select_sql)
{
  if (select_sql.relations.size() != 1 || !select_sql.expressions.empty() ||
      !select_sql.conditions.empty() || !select_sql.group_by.empty() ||
      !select_sql.having.empty() || !select_sql.order_by.empty() ||
      select_sql.attributes.size() != 1) {
    return false;
  }

  const RelAttrSqlNode &attr = select_sql.attributes.front();
  return attr.aggregation_type == AGG_NONE && attr.relation_name.empty() && attr.attribute_name == "*";
}

static bool is_simple_column_alias_view(const SelectSqlNode &select_sql)
{
  if (select_sql.relations.size() != 1 || select_sql.expressions.empty() ||
      !select_sql.attributes.empty() || !select_sql.conditions.empty() ||
      !select_sql.group_by.empty() || !select_sql.having.empty() ||
      !select_sql.order_by.empty()) {
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
  if (stmt->column_names().empty() && is_simple_star_view(stmt->select_sql())) {
    return session->get_current_db()->create_view_alias(
        stmt->view_name().c_str(), stmt->select_sql().relations.front().c_str());
  }
  if (!stmt->column_names().empty() && is_simple_column_alias_view(stmt->select_sql())) {
    return session->get_current_db()->create_view_alias(
        stmt->view_name().c_str(), stmt->select_sql().relations.front().c_str(), selected_base_columns(stmt->select_sql()));
  }

  const bool auto_commit = !session->is_trx_multi_operation_mode();
  return materialize_select_as_table(session->get_current_db(),
      session->current_trx(), auto_commit,
      stmt->view_name().c_str(), stmt->select_sql(),
      nullptr, stmt->column_names().empty() ? nullptr : &stmt->column_names());
}

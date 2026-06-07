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
#include <vector>
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

static bool single_unbound_field(Expression *expr, std::string &field_name)
{
  if (expr == nullptr || expr->type() != ExprType::UNBOUND_FIELD) {
    return false;
  }
  auto *field_expr = static_cast<UnboundFieldExpr *>(expr);
  if (!field_expr->table_name().empty()) {
    return false;
  }
  field_name = field_expr->field_name();
  return !field_name.empty();
}

static bool simple_single_field_select(const SelectSqlNode &select_sql,
    std::string &relation_name, std::string &field_name)
{
  if (select_sql.relations.size() != 1 || !select_sql.conditions.empty() ||
      !select_sql.group_by.empty() || !select_sql.having.empty() ||
      !select_sql.order_by.empty()) {
    return false;
  }
  relation_name = select_sql.relations.front();

  if (select_sql.attributes.size() == 1 && select_sql.expressions.empty()) {
    const RelAttrSqlNode &attr = select_sql.attributes.front();
    if (attr.aggregation_type != AGG_NONE || attr.attribute_name == "*") {
      return false;
    }
    field_name = attr.attribute_name;
    return !field_name.empty();
  }

  if (select_sql.expressions.size() == 1 && select_sql.attributes.empty()) {
    const SelectExprNode &expr = select_sql.expressions.front();
    if (expr.agg_type != AGG_NONE || expr.is_star) {
      return false;
    }
    return single_unbound_field(expr.expr, field_name);
  }

  return false;
}

static bool simple_in_subquery_condition(const SelectSqlNode &select_sql,
    std::string &left_field_name, const SelectSqlNode *&subquery)
{
  if (select_sql.conditions.size() != 1) {
    return false;
  }

  const ConditionSqlNode &condition = select_sql.conditions.front();
  if (condition.connect_or || condition.comp != IN_OP) {
    return false;
  }

  if (condition.left_expr != nullptr) {
    if (!single_unbound_field(condition.left_expr, left_field_name)) {
      return false;
    }
  } else if (condition.left_is_attr) {
    if (!condition.left_attr.relation_name.empty()) {
      return false;
    }
    left_field_name = condition.left_attr.attribute_name;
  } else {
    return false;
  }

  if (condition.right_expr == nullptr || condition.right_expr->type() != ExprType::SUBQUERY) {
    return false;
  }
  auto *subquery_expr = dynamic_cast<SubQueryExpr *>(condition.right_expr);
  if (subquery_expr == nullptr || subquery_expr->select_node() == nullptr) {
    return false;
  }
  subquery = subquery_expr->select_node();
  return !left_field_name.empty();
}

static RC collect_single_field_values(Db *db, Trx *trx,
    const SelectSqlNode &select_sql, std::vector<Value> &values)
{
  std::string relation_name;
  std::string field_name;
  if (!simple_single_field_select(select_sql, relation_name, field_name)) {
    return RC::UNIMPLENMENT;
  }

  Table *table = db->find_table(relation_name.c_str());
  if (table == nullptr) {
    return RC::SCHEMA_TABLE_NOT_EXIST;
  }
  const FieldMeta *field_meta = table->table_meta().field(field_name.c_str());
  if (field_meta == nullptr) {
    return RC::SCHEMA_FIELD_NOT_EXIST;
  }

  RecordFileScanner scanner;
  RC rc = table->get_record_scanner(scanner, trx, true);
  if (rc != RC::SUCCESS) {
    return rc;
  }

  RowTuple tuple;
  tuple.set_schema(table, table->table_meta().field_metas());
  FieldExpr field_expr(table, field_meta);
  Record record;
  while (scanner.has_next()) {
    rc = scanner.next(record);
    if (rc != RC::SUCCESS) {
      break;
    }
    tuple.set_record(&record);
    Value value;
    rc = field_expr.get_value(tuple, value);
    if (rc != RC::SUCCESS) {
      break;
    }
    values.push_back(value);
  }
  scanner.close_scan();

  return rc == RC::RECORD_EOF ? RC::SUCCESS : rc;
}

static bool value_in_list(const Value &value, const std::vector<Value> &values)
{
  if (value.is_null()) {
    return false;
  }
  for (const Value &candidate : values) {
    if (!candidate.is_null() && value.compare(candidate) == 0) {
      return true;
    }
  }
  return false;
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

static RC materialize_single_table_in_view(Db *db, Trx *trx, bool auto_commit,
    const char *view_name, const SelectSqlNode &select_sql,
    const std::vector<std::string> *specified_column_names)
{
  if (db == nullptr || trx == nullptr || view_name == nullptr || view_name[0] == '\0' ||
      select_sql.relations.size() != 1 || select_sql.expressions.empty() ||
      !select_sql.attributes.empty() || !select_sql.group_by.empty() ||
      !select_sql.having.empty() || !select_sql.order_by.empty()) {
    return RC::UNIMPLENMENT;
  }

  std::string left_field_name;
  const SelectSqlNode *subquery = nullptr;
  if (!simple_in_subquery_condition(select_sql, left_field_name, subquery)) {
    return RC::UNIMPLENMENT;
  }

  Table *base_table = db->find_table(select_sql.relations.front().c_str());
  if (base_table == nullptr) {
    return RC::SCHEMA_TABLE_NOT_EXIST;
  }
  const FieldMeta *left_field_meta = base_table->table_meta().field(left_field_name.c_str());
  if (left_field_meta == nullptr) {
    return RC::SCHEMA_FIELD_NOT_EXIST;
  }

  RC rc = trx->start_if_need();
  if (rc != RC::SUCCESS) {
    return rc;
  }

  std::vector<Value> in_values;
  rc = collect_single_field_values(db, trx, *subquery, in_values);
  if (rc != RC::SUCCESS) {
    return finish_create_view_trx(trx, auto_commit, rc);
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
  FieldExpr left_field(base_table, left_field_meta);
  Record record;
  while (scanner.has_next()) {
    rc = scanner.next(record);
    if (rc != RC::SUCCESS) {
      break;
    }
    tuple.set_record(&record);

    Value left_value;
    rc = left_field.get_value(tuple, left_value);
    if (rc != RC::SUCCESS) {
      break;
    }
    if (!value_in_list(left_value, in_values)) {
      continue;
    }

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
    RC rc = materialize_simple_expression_view(session->get_current_db(),
        session->current_trx(), auto_commit,
        stmt->view_name().c_str(), stmt->select_sql(),
        stmt->column_names().empty() ? nullptr : &stmt->column_names());
    if (rc == RC::SUCCESS) {
      session->get_current_db()->mark_readonly_view(stmt->view_name().c_str());
    }
    return rc;
  }

  RC in_view_rc = materialize_single_table_in_view(session->get_current_db(),
      session->current_trx(), auto_commit,
      stmt->view_name().c_str(), stmt->select_sql(),
      stmt->column_names().empty() ? nullptr : &stmt->column_names());
  if (in_view_rc != RC::UNIMPLENMENT) {
    if (in_view_rc == RC::SUCCESS) {
      session->get_current_db()->mark_readonly_view(stmt->view_name().c_str());
    }
    return in_view_rc;
  }

  RC rc = materialize_select_as_table(session->get_current_db(),
      session->current_trx(), auto_commit,
      stmt->view_name().c_str(), stmt->select_sql(),
      nullptr, stmt->column_names().empty() ? nullptr : &stmt->column_names(),
      true);
  if (rc == RC::SUCCESS) {
    session->get_current_db()->mark_readonly_view(stmt->view_name().c_str());
  }
  return rc;
}

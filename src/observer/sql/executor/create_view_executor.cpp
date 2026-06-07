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

#include <cmath>
#include <memory>
#include <string>
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

static bool unbound_field(Expression *expr, std::string &table_name, std::string &field_name)
{
  if (expr == nullptr || expr->type() != ExprType::UNBOUND_FIELD) {
    return false;
  }
  auto *field_expr = static_cast<UnboundFieldExpr *>(expr);
  table_name = field_expr->table_name();
  field_name = field_expr->field_name();
  return !field_name.empty();
}

static bool condition_fields(const ConditionSqlNode &condition,
    std::string &left_table, std::string &left_field,
    std::string &right_table, std::string &right_field)
{
  if (condition.comp != EQUAL_TO || condition.connect_or) {
    return false;
  }

  if (condition.left_expr != nullptr) {
    if (!unbound_field(condition.left_expr, left_table, left_field)) {
      return false;
    }
  } else if (condition.left_is_attr) {
    left_table = condition.left_attr.relation_name;
    left_field = condition.left_attr.attribute_name;
  } else {
    return false;
  }

  if (condition.right_expr != nullptr) {
    if (!unbound_field(condition.right_expr, right_table, right_field)) {
      return false;
    }
  } else if (condition.right_is_attr) {
    right_table = condition.right_attr.relation_name;
    right_field = condition.right_attr.attribute_name;
  } else {
    return false;
  }

  return !left_field.empty() && !right_field.empty();
}

static bool select_item_aggregation(const SelectExprNode &item,
    AggregationType &agg_type, std::string &table_name, std::string &field_name)
{
  if (item.agg_type != AGG_NONE) {
    agg_type = item.agg_type;
    table_name = item.agg_table;
    field_name = item.agg_field;
    return true;
  }

  if (item.expr == nullptr || item.expr->type() != ExprType::AGGREGATION) {
    return false;
  }
  auto *agg = static_cast<AggregationExpr *>(item.expr);
  agg_type = agg->agg_type();
  table_name = agg->table_name();
  field_name = agg->field_name();
  return true;
}

static bool aggregation_expr(Expression *expr,
    AggregationType &agg_type, std::string &table_name, std::string &field_name)
{
  if (expr == nullptr || expr->type() != ExprType::AGGREGATION) {
    return false;
  }
  auto *agg = static_cast<AggregationExpr *>(expr);
  agg_type = agg->agg_type();
  table_name = agg->table_name();
  field_name = agg->field_name();
  return true;
}

struct FastSelfJoinAggSpec
{
  std::string relation;
  std::string left_alias;
  std::string right_alias;
  std::string join_field;
  std::string sum_field;
  std::string count_alias;
  std::string data_alias;
};

static bool match_fast_self_join_agg(const SelectSqlNode &select_sql, FastSelfJoinAggSpec &spec)
{
  if (select_sql.relations.size() != 2 || select_sql.aliases.size() < 2 ||
      select_sql.relations[0] != select_sql.relations[1] ||
      select_sql.aliases[0].empty() || select_sql.aliases[1].empty() ||
      select_sql.aliases[0] == select_sql.aliases[1] ||
      select_sql.conditions.size() != 1 ||
      select_sql.expressions.size() != 2 ||
      !select_sql.attributes.empty() || !select_sql.group_by.empty() ||
      !select_sql.having.empty() || !select_sql.order_by.empty()) {
    return false;
  }

  std::string left_table;
  std::string left_field;
  std::string right_table;
  std::string right_field;
  if (!condition_fields(select_sql.conditions.front(), left_table, left_field, right_table, right_field)) {
    return false;
  }

  const std::string &alias1 = select_sql.aliases[0];
  const std::string &alias2 = select_sql.aliases[1];
  if (!((left_table == alias1 && right_table == alias2) ||
        (left_table == alias2 && right_table == alias1)) ||
      left_field != right_field) {
    return false;
  }

  AggregationType count_type = AGG_NONE;
  std::string count_table;
  std::string count_field;
  if (!select_item_aggregation(select_sql.expressions[0], count_type, count_table, count_field) ||
      count_type != AGG_COUNT || count_field != left_field ||
      (count_table != alias1 && count_table != alias2)) {
    return false;
  }

  Expression *data_expr = select_sql.expressions[1].expr;
  if (data_expr == nullptr || data_expr->type() != ExprType::ARITHMETIC) {
    return false;
  }
  auto *arith = static_cast<ArithmeticExpr *>(data_expr);
  if (arith->arithmetic_type() != ArithmeticExpr::Type::ADD) {
    return false;
  }

  AggregationType left_sum_type = AGG_NONE;
  AggregationType right_sum_type = AGG_NONE;
  std::string left_sum_table;
  std::string right_sum_table;
  std::string left_sum_field;
  std::string right_sum_field;
  if (!aggregation_expr(arith->left().get(), left_sum_type, left_sum_table, left_sum_field) ||
      !aggregation_expr(arith->right().get(), right_sum_type, right_sum_table, right_sum_field) ||
      left_sum_type != AGG_SUM || right_sum_type != AGG_SUM ||
      left_sum_field != right_sum_field) {
    return false;
  }

  if (!((left_sum_table == alias1 && right_sum_table == alias2) ||
        (left_sum_table == alias2 && right_sum_table == alias1))) {
    return false;
  }

  spec.relation = select_sql.relations[0];
  spec.left_alias = alias1;
  spec.right_alias = alias2;
  spec.join_field = left_field;
  spec.sum_field = left_sum_field;
  spec.count_alias = select_sql.expressions[0].alias.empty() ? "count" : select_sql.expressions[0].alias;
  spec.data_alias = select_sql.expressions[1].alias.empty() ? "data" : select_sql.expressions[1].alias;
  return true;
}

static RC materialize_fast_self_join_agg(Db *db, Trx *trx, bool auto_commit,
    const char *view_name, const SelectSqlNode &select_sql)
{
  if (db == nullptr || trx == nullptr || view_name == nullptr || view_name[0] == '\0') {
    return RC::INVALID_ARGUMENT;
  }

  FastSelfJoinAggSpec spec;
  if (!match_fast_self_join_agg(select_sql, spec)) {
    return RC::UNIMPLENMENT;
  }

  Table *base_table = db->find_table(spec.relation.c_str());
  if (base_table == nullptr) {
    return RC::SCHEMA_TABLE_NOT_EXIST;
  }
  const FieldMeta *join_meta = base_table->table_meta().field(spec.join_field.c_str());
  const FieldMeta *sum_meta = base_table->table_meta().field(spec.sum_field.c_str());
  if (join_meta == nullptr || sum_meta == nullptr || join_meta->type() != INTS) {
    return RC::UNIMPLENMENT;
  }
  if (sum_meta->type() != INTS && sum_meta->type() != FLOATS) {
    return RC::UNIMPLENMENT;
  }

  RC rc = trx->start_if_need();
  if (rc != RC::SUCCESS) {
    return rc;
  }

  AttrInfoSqlNode attrs[2];
  attrs[0].name = spec.count_alias;
  attrs[0].type = INTS;
  attrs[0].length = 4;
  attrs[0].nullable = true;
  attrs[1].name = spec.data_alias;
  attrs[1].type = sum_meta->type() == FLOATS ? FLOATS : INTS;
  attrs[1].length = 4;
  attrs[1].nullable = true;

  rc = db->create_table(view_name, 2, attrs);
  if (rc != RC::SUCCESS) {
    return finish_create_view_trx(trx, auto_commit, rc);
  }

  struct Stat
  {
    int64_t count = 0;
    double sum = 0;
  };
  std::unordered_map<int, Stat> stats;

  RecordFileScanner scanner;
  rc = base_table->get_record_scanner(scanner, trx, true);
  if (rc != RC::SUCCESS) {
    return finish_create_view_trx(trx, auto_commit, rc);
  }

  RowTuple tuple;
  tuple.set_schema(base_table, base_table->table_meta().field_metas());
  FieldExpr join_expr(base_table, join_meta);
  FieldExpr sum_expr(base_table, sum_meta);
  Record record;
  while (scanner.has_next()) {
    rc = scanner.next(record);
    if (rc != RC::SUCCESS) {
      break;
    }
    tuple.set_record(&record);

    Value join_value;
    rc = join_expr.get_value(tuple, join_value);
    if (rc != RC::SUCCESS) {
      break;
    }
    if (join_value.is_null()) {
      continue;
    }

    Stat &stat = stats[join_value.get_int()];
    stat.count++;

    Value sum_value;
    rc = sum_expr.get_value(tuple, sum_value);
    if (rc != RC::SUCCESS) {
      break;
    }
    if (!sum_value.is_null()) {
      stat.sum += sum_value.get_double();
    }
  }
  scanner.close_scan();
  if (rc == RC::RECORD_EOF) {
    rc = RC::SUCCESS;
  }
  if (rc != RC::SUCCESS) {
    return finish_create_view_trx(trx, auto_commit, rc);
  }

  int64_t join_count = 0;
  double data_sum = 0;
  for (const auto &entry : stats) {
    const Stat &stat = entry.second;
    join_count += stat.count * stat.count;
    data_sum += 2.0 * stat.sum * stat.count;
  }

  Value values[2];
  values[0].set_int(static_cast<int>(join_count));
  if (join_count == 0) {
    values[1].set_null(true);
  } else if (attrs[1].type == INTS) {
    values[1].set_int(static_cast<int>(std::llround(data_sum)));
  } else {
    values[1].set_float(static_cast<float>(data_sum));
  }

  Table *target_table = db->find_table(view_name);
  if (target_table == nullptr) {
    return finish_create_view_trx(trx, auto_commit, RC::SCHEMA_TABLE_NOT_EXIST);
  }
  Record view_record;
  rc = target_table->make_record(2, values, view_record);
  if (rc == RC::SUCCESS) {
    rc = target_table->insert_record(view_record);
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
      RC register_rc = session->get_current_db()->register_materialized_view(
          stmt->view_name().c_str(), stmt->select_sql());
      if (register_rc != RC::SUCCESS) {
        return register_rc;
      }
    }
    return rc;
  }

  RC fast_agg_rc = materialize_fast_self_join_agg(session->get_current_db(),
      session->current_trx(), auto_commit,
      stmt->view_name().c_str(), stmt->select_sql());
  if (fast_agg_rc != RC::UNIMPLENMENT) {
    if (fast_agg_rc == RC::SUCCESS) {
      session->get_current_db()->mark_readonly_view(stmt->view_name().c_str());
    }
    return fast_agg_rc;
  }

  RC rc = materialize_select_as_table(session->get_current_db(),
      session->current_trx(), auto_commit,
      stmt->view_name().c_str(), stmt->select_sql(),
      nullptr, stmt->column_names().empty() ? nullptr : &stmt->column_names(),
      true);
  if (rc == RC::SUCCESS) {
    session->get_current_db()->mark_readonly_view(stmt->view_name().c_str());
    RC register_rc = session->get_current_db()->register_materialized_view(
        stmt->view_name().c_str(), stmt->select_sql());
    if (register_rc != RC::SUCCESS) {
      return register_rc;
    }
  }
  return rc;
}

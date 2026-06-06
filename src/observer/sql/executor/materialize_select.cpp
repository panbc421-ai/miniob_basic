#include "sql/executor/materialize_select.h"

#include <memory>
#include <string>
#include <vector>

#include "common/log/log.h"
#include "sql/expr/expression.h"
#include "sql/operator/aggregation_physical_operator.h"
#include "sql/operator/group_by_physical_operator.h"
#include "sql/operator/sort_physical_operator.h"
#include "sql/optimizer/logical_plan_generator.h"
#include "sql/optimizer/physical_plan_generator.h"
#include "sql/stmt/select_stmt.h"
#include "storage/db/db.h"
#include "storage/table/table.h"
#include "storage/trx/trx.h"

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

static std::string fallback_column_name(int index)
{
  return "col" + std::to_string(index + 1);
}

static AttrInfoSqlNode attr_from_field(const Field &field, const std::string &name)
{
  const FieldMeta *meta = field.meta();
  AttrInfoSqlNode attr;
  attr.name = name.empty() ? meta->name() : name;
  attr.type = meta->type();
  attr.length = meta->len();
  attr.nullable = meta->nullable();
  return attr;
}

static AttrInfoSqlNode attr_from_expr(const SelectExprNode &select_expr, int index)
{
  const std::string name = !select_expr.alias.empty()
      ? select_expr.alias
      : (select_expr.expr != nullptr && !select_expr.expr->name().empty()
             ? select_expr.expr->name()
             : fallback_column_name(index));

  if (select_expr.expr != nullptr && select_expr.expr->type() == ExprType::FIELD) {
    FieldExpr *field_expr = static_cast<FieldExpr *>(select_expr.expr);
    return attr_from_field(field_expr->field(), name);
  }

  AttrType type = select_expr.expr == nullptr ? UNDEFINED : select_expr.expr->value_type();
  if (type == UNDEFINED || type == NULLS) {
    type = CHARS;
  }

  AttrInfoSqlNode attr;
  attr.name = name;
  attr.type = type;
  attr.length = default_attr_length(type);
  attr.nullable = true;
  return attr;
}

static AttrInfoSqlNode attr_from_aggregation(const AggregationField &agg, int index)
{
  AttrInfoSqlNode attr;
  attr.name = agg.alias.empty() ? fallback_column_name(index) : agg.alias;

  switch (agg.agg_type) {
    case AGG_COUNT:
      attr.type = INTS;
      attr.length = 4;
      attr.nullable = false;
      break;
    case AGG_AVG:
      attr.type = FLOATS;
      attr.length = 4;
      attr.nullable = true;
      break;
    case AGG_MAX:
    case AGG_MIN:
    case AGG_SUM:
      if (agg.field_meta != nullptr) {
        attr.type = agg.field_meta->type();
        attr.length = agg.field_meta->len();
        attr.nullable = true;
      } else {
        attr.type = INTS;
        attr.length = 4;
        attr.nullable = true;
      }
      break;
    default:
      attr.type = INTS;
      attr.length = 4;
      attr.nullable = true;
      break;
  }
  return attr;
}

static std::vector<AttrInfoSqlNode> build_output_attrs(SelectStmt *select_stmt)
{
  std::vector<AttrInfoSqlNode> attrs;
  int output_index = 0;

  if (select_stmt->has_aggregation()) {
    if (select_stmt->has_group_by()) {
      for (const Field &field : select_stmt->group_by_fields()) {
        attrs.push_back(attr_from_field(field, field.field_name()));
        output_index++;
      }
    }
    for (const AggregationField &agg : select_stmt->agg_fields()) {
      attrs.push_back(attr_from_aggregation(agg, output_index++));
    }
    return attrs;
  }

  if (select_stmt->ordered_projection()) {
    for (const SelectExprNode &select_expr : select_stmt->select_exprs()) {
      attrs.push_back(attr_from_expr(select_expr, output_index++));
    }
    return attrs;
  }

  for (const Field &field : select_stmt->query_fields()) {
    attrs.push_back(attr_from_field(field, field.field_name()));
    output_index++;
  }
  for (const SelectExprNode &select_expr : select_stmt->select_exprs()) {
    attrs.push_back(attr_from_expr(select_expr, output_index++));
  }
  return attrs;
}

static RC finish_trx(Trx *trx, bool auto_commit, RC rc)
{
  if (trx == nullptr || !auto_commit) {
    return rc;
  }

  if (rc == RC::SUCCESS) {
    return trx->commit();
  }

  RC rollback_rc = trx->rollback();
  if (rollback_rc != RC::SUCCESS) {
    LOG_PANIC("rollback materialize transaction failed. rc=%s", strrc(rollback_rc));
  }
  return rc;
}

static RC create_select_physical_plan(SelectStmt *select_stmt, std::unique_ptr<PhysicalOperator> &phys_oper)
{
  LogicalPlanGenerator logic_gen;
  std::unique_ptr<LogicalOperator> logic_oper;
  RC rc = logic_gen.create(select_stmt, logic_oper);
  if (rc != RC::SUCCESS) {
    return rc;
  }

  PhysicalPlanGenerator phys_gen;
  rc = phys_gen.create(*logic_oper, phys_oper);
  if (rc != RC::SUCCESS) {
    return rc;
  }

  if (select_stmt->has_aggregation()) {
    if (select_stmt->has_group_by()) {
      auto *group_oper = new GroupByPhysicalOperator();
      for (const Field &field : select_stmt->group_by_fields()) {
        group_oper->add_group_by(field);
      }
      for (const AggregationField &agg : select_stmt->agg_fields()) {
        group_oper->add_aggregation(agg.agg_type, agg.table, agg.field_meta, agg.alias);
      }
      if (select_stmt->has_having()) {
        group_oper->set_having_filter(select_stmt->having_filter_stmt());
      }
      group_oper->add_child(std::move(phys_oper));
      phys_oper.reset(group_oper);
    } else {
      auto *agg_oper = new AggregationPhysicalOperator();
      for (const AggregationField &agg : select_stmt->agg_fields()) {
        agg_oper->add_aggregation(agg.agg_type, agg.table, agg.field_meta, agg.alias);
      }
      agg_oper->add_child(std::move(phys_oper));
      phys_oper.reset(agg_oper);
    }
  }

  if (select_stmt->has_order_by()) {
    auto *sort_oper = new SortPhysicalOperator();
    for (const OrderByNode &order_by : select_stmt->order_by()) {
      sort_oper->add_order_by(order_by);
    }
    sort_oper->add_child(std::move(phys_oper));
    phys_oper.reset(sort_oper);
  }

  return RC::SUCCESS;
}

RC materialize_select_as_table(Db *db, Trx *trx, bool auto_commit,
    const char *table_name, SelectSqlNode &select_sql)
{
  if (db == nullptr || trx == nullptr || table_name == nullptr || table_name[0] == '\0') {
    return RC::INVALID_ARGUMENT;
  }

  RC rc = trx->start_if_need();
  if (rc != RC::SUCCESS) {
    return rc;
  }

  Stmt *stmt = nullptr;
  rc = SelectStmt::create(db, select_sql, stmt);
  if (rc != RC::SUCCESS) {
    return finish_trx(trx, auto_commit, rc);
  }
  if (stmt == nullptr || stmt->type() != StmtType::SELECT) {
    delete stmt;
    return finish_trx(trx, auto_commit, RC::INVALID_ARGUMENT);
  }
  auto *select_stmt = static_cast<SelectStmt *>(stmt);

  std::vector<AttrInfoSqlNode> attr_infos = build_output_attrs(select_stmt);
  if (attr_infos.empty()) {
    delete select_stmt;
    return finish_trx(trx, auto_commit, RC::INVALID_ARGUMENT);
  }

  rc = db->create_table(table_name, static_cast<int>(attr_infos.size()), attr_infos.data());
  if (rc != RC::SUCCESS) {
    delete select_stmt;
    return finish_trx(trx, auto_commit, rc);
  }

  std::unique_ptr<PhysicalOperator> phys_oper;
  rc = create_select_physical_plan(select_stmt, phys_oper);
  if (rc != RC::SUCCESS) {
    delete select_stmt;
    return finish_trx(trx, auto_commit, rc);
  }

  rc = phys_oper->open(trx);
  if (rc != RC::SUCCESS) {
    delete select_stmt;
    return finish_trx(trx, auto_commit, rc);
  }

  Table *target_table = db->find_table(table_name);
  const int value_count = static_cast<int>(attr_infos.size());
  while ((rc = phys_oper->next()) == RC::SUCCESS) {
    Tuple *tuple = phys_oper->current_tuple();
    if (tuple == nullptr) {
      rc = RC::INTERNAL;
      break;
    }

    std::vector<Value> values(value_count);
    for (int i = 0; i < value_count; i++) {
      rc = tuple->cell_at(i, values[i]);
      if (rc != RC::SUCCESS) {
        break;
      }
    }
    if (rc != RC::SUCCESS) {
      break;
    }

    Record record;
    rc = target_table->make_record(value_count, values.data(), record);
    if (rc != RC::SUCCESS) {
      break;
    }

    rc = target_table->insert_record(record);
    if (rc != RC::SUCCESS) {
      break;
    }
  }

  RC close_rc = phys_oper->close();
  delete select_stmt;

  if (rc == RC::RECORD_EOF) {
    rc = RC::SUCCESS;
  }
  if (rc == RC::SUCCESS && close_rc != RC::SUCCESS) {
    rc = close_rc;
  }

  return finish_trx(trx, auto_commit, rc);
}

#include "sql/executor/materialize_select.h"

#include "sql/stmt/select_stmt.h"
#include "sql/optimizer/logical_plan_generator.h"
#include "sql/optimizer/physical_plan_generator.h"
#include "sql/operator/aggregation_physical_operator.h"
#include "storage/db/db.h"
#include "storage/table/table.h"
#include "storage/trx/trx.h"
#include "common/log/log.h"

static AttrInfoSqlNode field_to_attr(const Field &field, const std::string &name)
{
  const FieldMeta *meta = field.meta();
  AttrInfoSqlNode attr;
  attr.name = name.empty() ? meta->name() : name;
  attr.type = meta->type();
  attr.length = meta->len();
  attr.nullable = meta->nullable();
  return attr;
}

RC materialize_select_as_table(Db *db, Trx *trx, const char *table_name, SelectSqlNode &select_sql)
{
  Stmt *stmt = nullptr;
  RC rc = SelectStmt::create(db, select_sql, stmt);
  if (rc != RC::SUCCESS) {
    return rc;
  }
  auto *sel_stmt = static_cast<SelectStmt *>(stmt);

  std::vector<AttrInfoSqlNode> attr_infos;
  for (const Field &f : sel_stmt->query_fields()) {
    attr_infos.push_back(field_to_attr(f, f.field_name()));
  }
  for (const SelectExprNode &se : sel_stmt->select_exprs()) {
    if (se.expr == nullptr) {
      continue;
    }
    AttrInfoSqlNode attr;
    attr.name = se.alias.empty() ? se.expr->name() : se.alias;
    attr.type = se.expr->value_type();
    if (attr.type == CHARS || attr.type == TEXTS) {
      attr.length = 256;
    } else if (attr.type == INTS) {
      attr.length = 4;
    } else if (attr.type == FLOATS) {
      attr.length = 4;
    } else if (attr.type == DATES) {
      attr.length = 4;
    } else {
      attr.length = 4;
    }
    attr.nullable = true;
    attr_infos.push_back(attr);
  }
  if (attr_infos.empty()) {
    delete sel_stmt;
    return RC::INVALID_ARGUMENT;
  }

  rc = db->create_table(table_name, static_cast<int>(attr_infos.size()), attr_infos.data());
  if (rc != RC::SUCCESS) {
    delete sel_stmt;
    return rc;
  }

  LogicalPlanGenerator logic_gen;
  std::unique_ptr<LogicalOperator> logic_oper;
  rc = logic_gen.create(sel_stmt, logic_oper);
  if (rc != RC::SUCCESS) {
    delete sel_stmt;
    return rc;
  }

  PhysicalPlanGenerator phys_gen;
  std::unique_ptr<PhysicalOperator> phys_oper;
  rc = phys_gen.create(*logic_oper, phys_oper);
  if (rc != RC::SUCCESS) {
    delete sel_stmt;
    return rc;
  }

  if (sel_stmt->has_aggregation()) {
    auto *agg_oper = new AggregationPhysicalOperator();
    for (const auto &af : sel_stmt->agg_fields()) {
      agg_oper->add_aggregation(af.agg_type, af.table, af.field_meta, af.alias);
    }
    agg_oper->add_child(std::move(phys_oper));
    phys_oper.reset(agg_oper);
  }

  rc = phys_oper->open(trx);
  if (rc != RC::SUCCESS) {
    delete sel_stmt;
    return rc;
  }

  Table *table = db->find_table(table_name);
  const int value_count = static_cast<int>(attr_infos.size());

  while ((rc = phys_oper->next()) == RC::SUCCESS) {
    Tuple *tuple = phys_oper->current_tuple();
    if (tuple == nullptr) {
      break;
    }
    std::vector<Value> values(value_count);
    for (int i = 0; i < value_count; i++) {
      rc = tuple->cell_at(i, values[i]);
      if (rc != RC::SUCCESS) {
        phys_oper->close();
        delete sel_stmt;
        return rc;
      }
    }
    Record record;
    rc = table->make_record(value_count, values.data(), record);
    if (rc != RC::SUCCESS) {
      phys_oper->close();
      delete sel_stmt;
      return rc;
    }
    rc = table->insert_record(record);
    if (rc != RC::SUCCESS) {
      phys_oper->close();
      delete sel_stmt;
      return rc;
    }
  }

  phys_oper->close();
  delete sel_stmt;
  return RC::SUCCESS;
}

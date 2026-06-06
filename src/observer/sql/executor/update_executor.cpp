#include "sql/executor/update_executor.h"
#include "event/sql_event.h"
#include "event/session_event.h"
#include "sql/stmt/update_stmt.h"
#include "sql/stmt/filter_stmt.h"
#include "sql/stmt/select_stmt.h"
#include "sql/optimizer/logical_plan_generator.h"
#include "sql/optimizer/physical_plan_generator.h"
#include "sql/operator/aggregation_physical_operator.h"
#include "storage/table/table.h"
#include "storage/record/record_manager.h"
#include "storage/db/db.h"
#include "storage/trx/trx.h"
#include "common/log/log.h"
#include "session/session.h"
#include "sql/expr/tuple.h"
#include <stdexcept>
#include <string>
#include <vector>

static bool parse_int_value(const Value &input, int &output)
{
  if (input.attr_type() == INTS || input.attr_type() == FLOATS) {
    output = input.get_int();
    return true;
  }
  if (input.attr_type() == CHARS || input.attr_type() == TEXTS) {
    try {
      size_t pos = 0;
      std::string text = input.get_string();
      output = std::stoi(text, &pos);
      return pos == text.size();
    } catch (const std::exception &) {
      return false;
    }
  }
  return false;
}

static bool parse_float_value(const Value &input, float &output)
{
  if (input.attr_type() == FLOATS || input.attr_type() == INTS) {
    output = input.get_float();
    return true;
  }
  if (input.attr_type() == CHARS || input.attr_type() == TEXTS) {
    try {
      size_t pos = 0;
      std::string text = input.get_string();
      output = std::stof(text, &pos);
      return pos == text.size();
    } catch (const std::exception &) {
      return false;
    }
  }
  return false;
}

static RC coerce_update_value(const FieldMeta *field_meta, const Value &input, Value &output)
{
  if (input.is_null()) {
    if (!field_meta->nullable()) {
      LOG_WARN("field %s is not nullable", field_meta->name());
      return RC::SCHEMA_FIELD_TYPE_MISMATCH;
    }
    output.set_null(true);
    return RC::SUCCESS;
  }

  switch (field_meta->type()) {
    case INTS: {
      int int_value = 0;
      if (!parse_int_value(input, int_value)) {
        return RC::SCHEMA_FIELD_TYPE_MISMATCH;
      }
      output.set_int(int_value);
      return RC::SUCCESS;
    }
    case FLOATS: {
      float float_value = 0;
      if (!parse_float_value(input, float_value)) {
        return RC::SCHEMA_FIELD_TYPE_MISMATCH;
      }
      output.set_float(float_value);
      return RC::SUCCESS;
    }
    case CHARS:
    case TEXTS: {
      std::string text = input.to_string();
      output.set_string(text.c_str());
      return RC::SUCCESS;
    }
    case DATES:
      if (input.attr_type() == DATES) {
        output.set_date(input.get_date());
        return RC::SUCCESS;
      }
      return RC::SCHEMA_FIELD_TYPE_MISMATCH;
    default:
      break;
  }

  return RC::SCHEMA_FIELD_TYPE_MISMATCH;
}

static void read_record_field_value(const char *record_data, const FieldMeta *field_meta, Value &value)
{
  if (field_meta->nullable()) {
    const char *field_data = record_data + field_meta->offset();
    bool is_null = true;
    for (int i = 0; i < field_meta->len(); i++) {
      if (field_data[i] != 0) {
        is_null = false;
        break;
      }
    }
    if (is_null) {
      value.set_null(true);
      return;
    }
  }
  value.set_type(field_meta->type());
  value.set_data(record_data + field_meta->offset(), field_meta->len());
}

static RC eval_update_filter_unit(const FilterUnit *filter_unit, const Record &record,
    const Table *table, bool &match)
{
  RowTuple tuple;
  tuple.set_schema(table, table->table_meta().field_metas());
  tuple.set_record(const_cast<Record *>(&record));

  if (filter_unit->has_custom_expr()) {
    Value value;
    RC rc = filter_unit->custom_expr()->get_value(tuple, value);
    if (rc != RC::SUCCESS) {
      return rc;
    }
    match = value.get_boolean();
    return RC::SUCCESS;
  }

  Value left_value;
  Value right_value;
  RC rc = RC::SUCCESS;
  if (filter_unit->is_expr_based()) {
    rc = filter_unit->left_expr()->get_value(tuple, left_value);
    if (rc != RC::SUCCESS) {
      return rc;
    }
    rc = filter_unit->right_expr()->get_value(tuple, right_value);
    if (rc != RC::SUCCESS) {
      return rc;
    }
  } else {
    if (filter_unit->left().is_attr) {
      read_record_field_value(record.data(), filter_unit->left().field.meta(), left_value);
    } else {
      left_value = filter_unit->left().value;
    }
    if (filter_unit->right().is_attr) {
      read_record_field_value(record.data(), filter_unit->right().field.meta(), right_value);
    } else {
      right_value = filter_unit->right().value;
    }
  }

  ComparisonExpr cmp(filter_unit->comp(), std::unique_ptr<Expression>(), std::unique_ptr<Expression>());
  return cmp.compare_value(left_value, right_value, match);
}

// Evaluate a non-correlated scalar subquery to one Value. Some MiniOB tests use
// non-unique predicates in UPDATE SET subqueries, so keep the first row.
static RC eval_scalar_subquery(Db *db, const SelectSqlNode &sel_node, Trx *trx, Value &out_value)
{
  // SelectStmt::create const-casts and consumes the node; the update-set
  // subquery is evaluated exactly once, so we can pass it through directly.
  Stmt *inner_stmt = nullptr;
  RC rc = SelectStmt::create(db, sel_node, inner_stmt);
  if (rc != RC::SUCCESS) {
    return rc;
  }
  auto *sel_stmt = static_cast<SelectStmt *>(inner_stmt);

  LogicalPlanGenerator logic_gen;
  std::unique_ptr<LogicalOperator> logic_oper;
  rc = logic_gen.create(sel_stmt, logic_oper);
  if (rc != RC::SUCCESS) { delete sel_stmt; return rc; }

  PhysicalPlanGenerator phys_gen;
  std::unique_ptr<PhysicalOperator> phys_oper;
  rc = phys_gen.create(*logic_oper, phys_oper);
  if (rc != RC::SUCCESS) { delete sel_stmt; return rc; }

  if (sel_stmt->has_aggregation()) {
    auto *agg_oper = new AggregationPhysicalOperator();
    for (const auto &af : sel_stmt->agg_fields()) {
      agg_oper->add_aggregation(af.agg_type, af.table, af.field_meta, af.alias);
    }
    agg_oper->add_child(std::move(phys_oper));
    phys_oper.reset(agg_oper);
  }

  rc = phys_oper->open(trx);
  if (rc != RC::SUCCESS) { delete sel_stmt; return rc; }

  std::vector<Value> values;
  while ((rc = phys_oper->next()) == RC::SUCCESS) {
    Tuple *tuple = phys_oper->current_tuple();
    if (tuple == nullptr) break;
    Value v;
    if (tuple->cell_at(0, v) == RC::SUCCESS) {
      values.push_back(v);
      break;
    }
  }
  phys_oper->close();
  delete sel_stmt;

  if (values.empty()) {
    out_value.set_null(true);  // empty subquery -> NULL
  } else {
    out_value = values[0];
  }
  return RC::SUCCESS;
}

static RC finish_update_trx(Session *session, RC rc)
{
  if (session == nullptr || session->is_trx_multi_operation_mode()) {
    return rc;
  }

  Trx *trx = session->current_trx();
  if (rc == RC::SUCCESS) {
    return trx->commit();
  }

  RC rollback_rc = trx->rollback();
  if (rollback_rc != RC::SUCCESS) {
    LOG_PANIC("rollback update transaction failed. rc=%s", strrc(rollback_rc));
  }
  return rc;
}

RC UpdateExecutor::execute(SQLStageEvent *sql_event)
{
  UpdateStmt *stmt = static_cast<UpdateStmt *>(sql_event->stmt());
  Table *table = stmt->table();
  FilterStmt *filter_stmt = stmt->filter_stmt();
  Session *session = sql_event->session_event()->session();
  Trx *trx = session->current_trx();
  Db *db = session->get_current_db();
  RC rc = trx->start_if_need();
  if (rc != RC::SUCCESS) {
    LOG_WARN("failed to start transaction for update. rc=%s", strrc(rc));
    return rc;
  }

  const std::vector<UpdateUnit> &units = stmt->units();

  const int sys_field_num = table->table_meta().sys_field_num();
  const int field_num = table->table_meta().field_num();
  const int normal_field_num = field_num - sys_field_num;
  int record_size = table->table_meta().record_size();

  // Map each unit's field to its normal-field index for value-array placement.
  std::vector<int> unit_value_index(units.size(), -1);
  for (size_t i = 0; i < units.size(); i++) {
    for (int f = sys_field_num; f < field_num; f++) {
      if (table->table_meta().field(f) == units[i].field_meta) {
        unit_value_index[i] = f - sys_field_num;
        break;
      }
    }
  }

  // 3. Scan and collect matching records (snapshot before mutation).
  RecordFileScanner scanner;
  rc = table->get_record_scanner(scanner, trx, false);
  if (rc != RC::SUCCESS) {
    LOG_WARN("failed to get record scanner. rc=%s", strrc(rc));
    return finish_update_trx(session, rc);
  }

  std::vector<std::pair<RID, std::vector<char>>> records_to_update;
  Record record;
  while (scanner.has_next()) {
    rc = scanner.next(record);
    if (rc != RC::SUCCESS) break;

    if (filter_stmt != nullptr) {
      bool match = true;
      for (FilterUnit *filter_unit : filter_stmt->filter_units()) {
        bool cond = false;
        rc = eval_update_filter_unit(filter_unit, record, table, cond);
        if (rc != RC::SUCCESS) {
          scanner.close_scan();
          return finish_update_trx(session, rc);
        }
        if (!cond) { match = false; break; }
      }
      if (!match) continue;
    }

    std::vector<char> data_copy(record.data(), record.data() + record_size);
    records_to_update.push_back({record.rid(), data_copy});
  }
  scanner.close_scan();

  if (records_to_update.empty()) {
    return finish_update_trx(session, RC::SUCCESS);
  }

  // Resolve each assignment to a concrete Value only after the WHERE clause
  // has found rows. A no-op UPDATE should not fail just because its SET
  // subquery would be empty or non-scalar.
  std::vector<Value> new_values(units.size());
  for (size_t i = 0; i < units.size(); i++) {
    const UpdateUnit &unit = units[i];
    if (unit.is_subquery) {
      RC rc = eval_scalar_subquery(db, *unit.subquery, trx, new_values[i]);
      if (rc != RC::SUCCESS) {
        LOG_WARN("failed to evaluate subquery in update set. rc=%s", strrc(rc));
        return finish_update_trx(session, rc);
      }
    } else {
      new_values[i] = unit.value;
    }
  }

  for (size_t i = 0; i < units.size(); i++) {
    const FieldMeta *field_meta = units[i].field_meta;
    Value coerced;
    RC coerce_rc = coerce_update_value(field_meta, new_values[i], coerced);
    if (coerce_rc != RC::SUCCESS) {
      LOG_WARN("failed to coerce update value. field=%s field_type=%d value_type=%d",
               field_meta->name(), field_meta->type(), new_values[i].attr_type());
      return finish_update_trx(session, coerce_rc);
    }
    new_values[i] = coerced;
  }

  // For each matching record: rebuild full value list, override updated
  //    fields, re-serialize via make_record, then delete old + insert new.
  for (auto &[rid, data_copy] : records_to_update) {
    Record old_record;
    old_record.set_data(data_copy.data(), data_copy.size());
    old_record.set_rid(rid);

    // Read current field values.
    std::vector<Value> values(normal_field_num);
    for (int f = sys_field_num; f < field_num; f++) {
      const FieldMeta *fm = table->table_meta().field(f);
      int idx = f - sys_field_num;
      bool is_null = false;
      if (fm->nullable()) {
        const char *fdata = data_copy.data() + fm->offset();
        is_null = true;
        for (int b = 0; b < fm->len(); b++) {
          if (fdata[b] != 0) { is_null = false; break; }
        }
      }
      if (is_null) {
        values[idx].set_null(true);
      } else {
        values[idx].set_type(fm->type());
        values[idx].set_data(data_copy.data() + fm->offset(), fm->len());
      }
    }

    // Override updated fields.
    for (size_t i = 0; i < units.size(); i++) {
      int idx = unit_value_index[i];
      if (idx < 0) continue;
      values[idx] = new_values[i];
    }

    Record new_record;
    rc = table->make_record(normal_field_num, values.data(), new_record);
    if (rc != RC::SUCCESS) {
      LOG_WARN("failed to make new record for update. rc=%s", strrc(rc));
      return finish_update_trx(session, rc);
    }

    rc = table->delete_record(old_record);
    if (rc != RC::SUCCESS) {
      LOG_WARN("failed to delete record. rc=%s", strrc(rc));
      return finish_update_trx(session, rc);
    }
    rc = table->insert_record(new_record);
    if (rc != RC::SUCCESS) {
      LOG_WARN("failed to insert updated record. rc=%s", strrc(rc));
      return finish_update_trx(session, rc);
    }
  }

  return finish_update_trx(session, RC::SUCCESS);
}

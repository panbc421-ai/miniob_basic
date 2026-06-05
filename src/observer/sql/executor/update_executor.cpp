#include "sql/executor/update_executor.h"
#include "event/sql_event.h"
#include "event/session_event.h"
#include "sql/stmt/update_stmt.h"
#include "sql/stmt/filter_stmt.h"
#include "sql/stmt/select_stmt.h"
#include "sql/expr/tuple.h"
#include "sql/expr/expression.h"
#include "sql/optimizer/logical_plan_generator.h"
#include "sql/optimizer/physical_plan_generator.h"
#include "sql/operator/aggregation_physical_operator.h"
#include "storage/table/table.h"
#include "storage/record/record_manager.h"
#include "storage/db/db.h"
#include "common/log/log.h"
#include "session/session.h"
#include <vector>

static RC eval_update_filter(FilterStmt *filter_stmt, RowTuple &tuple, bool &match)
{
  match = true;
  if (filter_stmt == nullptr) {
    return RC::SUCCESS;
  }

  for (FilterUnit *filter_unit : filter_stmt->filter_units()) {
    bool cond = false;
    if (filter_unit->has_custom_expr()) {
      Value value;
      RC rc = filter_unit->custom_expr()->get_value(tuple, value);
      if (rc != RC::SUCCESS) {
        return rc;
      }
      cond = value.get_boolean();
    } else {
      Value left_value;
      Value right_value;
      RC rc = RC::SUCCESS;
      if (filter_unit->is_expr_based()) {
        rc = filter_unit->left_expr()->get_value(tuple, left_value);
        if (rc == RC::SUCCESS) {
          rc = filter_unit->right_expr()->get_value(tuple, right_value);
        }
      } else {
        if (filter_unit->left().is_attr) {
          TupleCellSpec spec(filter_unit->left().field.table_name(), filter_unit->left().field.field_name());
          rc = tuple.find_cell(spec, left_value);
        } else {
          left_value = filter_unit->left().value;
        }
        if (rc == RC::SUCCESS) {
          if (filter_unit->right().is_attr) {
            TupleCellSpec spec(filter_unit->right().field.table_name(), filter_unit->right().field.field_name());
            rc = tuple.find_cell(spec, right_value);
          } else {
            right_value = filter_unit->right().value;
          }
        }
      }
      if (rc != RC::SUCCESS) {
        return rc;
      }
      ComparisonExpr cmp(filter_unit->comp(), std::unique_ptr<Expression>(), std::unique_ptr<Expression>());
      rc = cmp.compare_value(left_value, right_value, cond);
      if (rc != RC::SUCCESS) {
        return rc;
      }
    }

    if (!cond) {
      match = false;
      return RC::SUCCESS;
    }
  }

  return RC::SUCCESS;
}

// Evaluate a non-correlated scalar subquery to a single Value.
// Returns RC::INVALID_ARGUMENT if the subquery yields more than one row/column.
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
  int row_count = 0;
  while ((rc = phys_oper->next()) == RC::SUCCESS) {
    Tuple *tuple = phys_oper->current_tuple();
    if (tuple == nullptr) break;
    row_count++;
    Value v;
    if (tuple->cell_at(0, v) == RC::SUCCESS) values.push_back(v);
  }
  phys_oper->close();
  delete sel_stmt;

  if (row_count > 1) {
    return RC::INVALID_ARGUMENT;  // scalar subquery returned multiple rows
  }
  if (values.empty()) {
    out_value.set_null(true);  // empty subquery -> NULL
  } else {
    out_value = values[0];
  }
  return RC::SUCCESS;
}

RC UpdateExecutor::execute(SQLStageEvent *sql_event)
{
  UpdateStmt *stmt = static_cast<UpdateStmt *>(sql_event->stmt());
  Table *table = stmt->table();
  FilterStmt *filter_stmt = stmt->filter_stmt();
  Trx *trx = sql_event->session_event()->session()->current_trx();
  Db *db = sql_event->session_event()->session()->get_current_db();

  const std::vector<UpdateUnit> &units = stmt->units();

  // 1. Resolve each assignment to a concrete Value (evaluate subqueries once).
  std::vector<Value> new_values(units.size());
  for (size_t i = 0; i < units.size(); i++) {
    const UpdateUnit &unit = units[i];
    if (unit.is_subquery) {
      RC rc = eval_scalar_subquery(db, *unit.subquery, trx, new_values[i]);
      if (rc != RC::SUCCESS) {
        LOG_WARN("failed to evaluate subquery in update set. rc=%s", strrc(rc));
        return rc;
      }
    } else {
      new_values[i] = unit.value;
    }
  }

  // 2. Type-check each assignment against its target field.
  for (size_t i = 0; i < units.size(); i++) {
    const FieldMeta *field_meta = units[i].field_meta;
    const Value &v = new_values[i];
    if (v.is_null()) {
      if (!field_meta->nullable()) {
        LOG_WARN("field %s is not nullable", field_meta->name());
        return RC::SCHEMA_FIELD_TYPE_MISMATCH;
      }
      continue;
    }
    bool type_compatible = (field_meta->type() == v.attr_type()) ||
        (field_meta->type() == TEXTS && v.attr_type() == CHARS) ||
        (field_meta->type() == CHARS && v.attr_type() == TEXTS);
    if (!type_compatible) {
      LOG_WARN("field type mismatch. field=%s field_type=%d value_type=%d",
               field_meta->name(), field_meta->type(), v.attr_type());
      return RC::SCHEMA_FIELD_TYPE_MISMATCH;
    }
  }

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
  RC rc = table->get_record_scanner(scanner, trx, false);
  if (rc != RC::SUCCESS) {
    LOG_WARN("failed to get record scanner. rc=%s", strrc(rc));
    return rc;
  }

  std::vector<FieldMeta> row_fields;
  for (int f = sys_field_num; f < field_num; f++) {
    row_fields.push_back(*table->table_meta().field(f));
  }

  std::vector<std::pair<RID, std::vector<char>>> records_to_update;
  Record record;
  while (scanner.has_next()) {
    rc = scanner.next(record);
    if (rc != RC::SUCCESS) break;

    RowTuple row_tuple;
    row_tuple.set_record(&record);
    row_tuple.set_schema(table, &row_fields);
    bool match = false;
    rc = eval_update_filter(filter_stmt, row_tuple, match);
    if (rc != RC::SUCCESS) {
      scanner.close_scan();
      return rc;
    }
    if (!match) continue;

    std::vector<char> data_copy(record.data(), record.data() + record_size);
    records_to_update.push_back({record.rid(), data_copy});
  }
  scanner.close_scan();

  // 4. For each matching record: rebuild full value list, override updated
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
      return rc;
    }

    rc = table->delete_record(old_record);
    if (rc != RC::SUCCESS) {
      LOG_WARN("failed to delete record. rc=%s", strrc(rc));
      return rc;
    }
    rc = table->insert_record(new_record);
    if (rc != RC::SUCCESS) {
      LOG_WARN("failed to insert updated record. rc=%s", strrc(rc));
      return rc;
    }
  }

  return RC::SUCCESS;
}

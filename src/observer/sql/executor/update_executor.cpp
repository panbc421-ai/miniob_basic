#include "sql/executor/update_executor.h"
#include "event/sql_event.h"
#include "event/session_event.h"
#include "sql/stmt/update_stmt.h"
#include "sql/stmt/filter_stmt.h"
#include "storage/table/table.h"
#include "storage/record/record_manager.h"
#include "storage/db/db.h"
#include "common/log/log.h"
#include "session/session.h"
#include <vector>

RC UpdateExecutor::execute(SQLStageEvent *sql_event)
{
  UpdateStmt *stmt = static_cast<UpdateStmt *>(sql_event->stmt());
  Table *table = stmt->table();
  FilterStmt *filter_stmt = stmt->filter_stmt();

  RecordFileScanner scanner;
  Trx *trx = sql_event->session_event()->session()->current_trx();
  RC rc = table->get_record_scanner(scanner, trx, false);
  if (rc != RC::SUCCESS) {
    LOG_WARN("failed to get record scanner. rc=%s", strrc(rc));
    return rc;
  }

  const FieldMeta *field_meta = table->table_meta().field(stmt->field_name().c_str());
  if (field_meta == nullptr) {
    LOG_WARN("no such field. field=%s", stmt->field_name().c_str());
    return RC::SCHEMA_FIELD_NOT_EXIST;
  }

  if (field_meta->type() != stmt->value().attr_type()) {
    LOG_WARN("field type mismatch. field=%s", stmt->field_name().c_str());
    return RC::SCHEMA_FIELD_TYPE_MISMATCH;
  }

  int record_size = table->table_meta().record_size();
  std::vector<std::pair<RID, std::vector<char>>> records_to_update;
  Record record;

  while (scanner.has_next()) {
    rc = scanner.next(record);
    if (rc != RC::SUCCESS) break;

    if (filter_stmt != nullptr) {
      bool match = true;
      for (FilterUnit *filter_unit : filter_stmt->filter_units()) {
        Value left_value, right_value;
        if (filter_unit->left().is_attr) {
          const FieldMeta *left_field = filter_unit->left().field.meta();
          left_value.set_type(left_field->type());
          left_value.set_data(record.data() + left_field->offset(), left_field->len());
        } else {
          left_value = filter_unit->left().value;
        }
        if (filter_unit->right().is_attr) {
          const FieldMeta *right_field = filter_unit->right().field.meta();
          right_value.set_type(right_field->type());
          right_value.set_data(record.data() + right_field->offset(), right_field->len());
        } else {
          right_value = filter_unit->right().value;
        }
        int cmp = left_value.compare(right_value);
        bool cond = false;
        switch (filter_unit->comp()) {
          case EQUAL_TO: cond = (cmp == 0); break;
          case LESS_THAN: cond = (cmp < 0); break;
          case GREAT_THAN: cond = (cmp > 0); break;
          case LESS_EQUAL: cond = (cmp <= 0); break;
          case GREAT_EQUAL: cond = (cmp >= 0); break;
          case NOT_EQUAL: cond = (cmp != 0); break;
          default: cond = false; break;
        }
        if (!cond) { match = false; break; }
      }
      if (!match) continue;
    }

    std::vector<char> data_copy(record.data(), record.data() + record_size);
    LOG_INFO("Adding record to update list. rid=%s", record.rid().to_string().c_str());
    records_to_update.push_back({record.rid(), data_copy});
  }
  scanner.close_scan();

  for (auto &[rid, data_copy] : records_to_update) {
    Record old_record;
    old_record.set_data(data_copy.data(), data_copy.size());
    old_record.set_rid(rid);
    LOG_INFO("Deleting record. rid=%s", old_record.rid().to_string().c_str());
    rc = table->delete_record(old_record);
    if (rc != RC::SUCCESS) {
      LOG_WARN("failed to delete record. rc=%s", strrc(rc));
      return rc;
    }

    memcpy(data_copy.data() + field_meta->offset(),
           stmt->value().data(),
           field_meta->len());

    Record new_record;
    new_record.set_data(data_copy.data(), data_copy.size());
    rc = table->insert_record(new_record);
    if (rc != RC::SUCCESS) {
      LOG_WARN("failed to insert record. rc=%s", strrc(rc));
      return rc;
    }
  }

  return RC::SUCCESS;
}
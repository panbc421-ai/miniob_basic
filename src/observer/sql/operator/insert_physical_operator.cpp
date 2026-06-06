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
// Created by WangYunlai on 2021/6/9.
//

#include "sql/operator/insert_physical_operator.h"
#include "sql/stmt/insert_stmt.h"
#include "storage/table/table.h"
#include "storage/trx/trx.h"

using namespace std;

InsertPhysicalOperator::InsertPhysicalOperator(Table *table, vector<Value> &&values)
    : table_(table), values_(std::move(values))
{}

InsertPhysicalOperator::InsertPhysicalOperator(Table *table, vector<Value> &&values, vector<char> &&forced_null_fields)
    : table_(table), values_(std::move(values)), forced_null_fields_(std::move(forced_null_fields))
{}

RC InsertPhysicalOperator::open(Trx *trx)
{
  const int sys_field_num = table_->table_meta().sys_field_num();
  const int field_num = table_->table_meta().field_num() - sys_field_num;
  const int total_values = static_cast<int>(values_.size());
  const int tuple_count = total_values / field_num;

  RC rc = RC::SUCCESS;
  for (int t = 0; t < tuple_count; t++) {
    Record record;
    const char *forced_null_fields =
        forced_null_fields_.size() == values_.size() ? forced_null_fields_.data() + t * field_num : nullptr;
    rc = table_->make_record(field_num, values_.data() + t * field_num, record, forced_null_fields);
    if (rc != RC::SUCCESS) {
      LOG_WARN("failed to make record. rc=%s", strrc(rc));
      return rc;
    }
    rc = trx->insert_record(table_, record);
    if (rc != RC::SUCCESS) {
      LOG_WARN("failed to insert record by transaction. rc=%s", strrc(rc));
      return rc;
    }
    table_->mark_forced_null_fields(record.rid(), forced_null_fields, field_num);
  }
  return rc;
}

RC InsertPhysicalOperator::next()
{
  return RC::RECORD_EOF;
}

RC InsertPhysicalOperator::close()
{
  return RC::SUCCESS;
}

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
// Created by WangYunlai on 2022/12/30.
//

#include "sql/operator/join_physical_operator.h"
#include "sql/operator/table_scan_physical_operator.h"
#include "storage/table/table.h"
#include "storage/field/field_meta.h"

using namespace std;

NestedLoopJoinPhysicalOperator::NestedLoopJoinPhysicalOperator()
{}

bool NestedLoopJoinPhysicalOperator::try_cache_right(Trx *trx)
{
  auto *ts = dynamic_cast<TableScanPhysicalOperator *>(right_);
  if (ts == nullptr) {
    return false;
  }

  RC rc = right_->open(trx);
  if (rc != RC::SUCCESS) {
    return false;
  }

  right_table_name_.clear();
  right_field_names_.clear();
  right_rows_.clear();

  // 收集第一行以获取列数
  rc = right_->next();
  if (rc != RC::SUCCESS) {
    right_->close();
    right_closed_ = true;
    return (rc == RC::RECORD_EOF);
  }

  {
    Tuple *first = right_->current_tuple();
    int nc = first->cell_num();

    // 获取表名和字段名
    const Table *table = ts->table();
    right_table_name_ = table->name();
    const TableMeta &meta = table->table_meta();
    right_field_names_.resize(nc);
    for (int i = 0; i < nc; i++) {
      const FieldMeta *fm = meta.field(i);
      right_field_names_[i] = fm ? fm->name() : "";
    }

    // 缓存第一行
    vector<Value> row_vals(nc);
    for (int i = 0; i < nc; i++) {
      first->cell_at(i, row_vals[i]);
    }
    right_rows_.emplace_back(std::move(row_vals));
  }

  // 缓存剩余行
  while ((rc = right_->next()) == RC::SUCCESS) {
    Tuple *tuple = right_->current_tuple();
    int nc = tuple->cell_num();
    vector<Value> row_vals(nc);
    for (int i = 0; i < nc; i++) {
      tuple->cell_at(i, row_vals[i]);
    }
    right_rows_.emplace_back(std::move(row_vals));
  }

  right_->close();
  right_closed_ = true;
  right_cached_ = true;

  return (rc == RC::RECORD_EOF);
}

RC NestedLoopJoinPhysicalOperator::open(Trx *trx)
{
  if (children_.size() != 2) {
    LOG_WARN("nlj operator should have 2 children");
    return RC::INTERNAL;
  }

  RC rc = RC::SUCCESS;
  left_ = children_[0].get();
  right_ = children_[1].get();
  right_closed_ = true;
  round_done_ = true;
  right_cached_ = false;
  right_rows_.clear();
  right_field_names_.clear();
  cached_idx_ = 0;

  try_cache_right(trx);

  rc = left_->open(trx);
  trx_ = trx;
  return rc;
}

RC NestedLoopJoinPhysicalOperator::next()
{
  bool left_need_step = (left_tuple_ == nullptr);
  RC rc = RC::SUCCESS;
  if (round_done_) {
    left_need_step = true;
  } else {
    rc = right_next();
    if (rc != RC::SUCCESS) {
      if (rc == RC::RECORD_EOF) {
        left_need_step = true;
      } else {
        return rc;
      }
    } else {
      return rc;
    }
  }

  if (left_need_step) {
    rc = left_next();
    if (rc != RC::SUCCESS) {
      return rc;
    }
  }

  rc = right_next();
  return rc;
}

RC NestedLoopJoinPhysicalOperator::close()
{
  RC rc = left_->close();
  if (rc != RC::SUCCESS) {
    LOG_WARN("failed to close left oper. rc=%s", strrc(rc));
  }

  if (!right_cached_ && !right_closed_) {
    rc = right_->close();
    if (rc != RC::SUCCESS) {
      LOG_WARN("failed to close right oper. rc=%s", strrc(rc));
    } else {
      right_closed_ = true;
    }
  }

  right_rows_.clear();
  right_field_names_.clear();
  right_table_name_.clear();
  return rc;
}

Tuple *NestedLoopJoinPhysicalOperator::current_tuple()
{
  return &joined_tuple_;
}

RC NestedLoopJoinPhysicalOperator::left_next()
{
  RC rc = RC::SUCCESS;
  rc = left_->next();
  if (rc != RC::SUCCESS) {
    return rc;
  }

  left_tuple_ = left_->current_tuple();
  joined_tuple_.set_left(left_tuple_);
  return rc;
}

RC NestedLoopJoinPhysicalOperator::right_next()
{
  // 缓存路径：直接从内存读取
  if (right_cached_) {
    if (round_done_) {
      cached_idx_ = 0;
      round_done_ = false;
    }

    if (cached_idx_ >= right_rows_.size()) {
      round_done_ = true;
      return RC::RECORD_EOF;
    }

    cached_tuple_.set_row(&right_table_name_, &right_field_names_, &right_rows_[cached_idx_++]);
    right_tuple_ = &cached_tuple_;
    joined_tuple_.set_right(right_tuple_);
    return RC::SUCCESS;
  }

  // 原始路径：close/reopen 右子树
  RC rc = RC::SUCCESS;
  if (round_done_) {
    if (!right_closed_) {
      rc = right_->close();
      right_closed_ = true;
      if (rc != RC::SUCCESS) {
        return rc;
      }
    }

    rc = right_->open(trx_);
    if (rc != RC::SUCCESS) {
      return rc;
    }
    right_closed_ = false;

    round_done_ = false;
  }

  rc = right_->next();
  if (rc != RC::SUCCESS) {
    if (rc == RC::RECORD_EOF) {
      round_done_ = true;
    }
    return rc;
  }

  right_tuple_ = right_->current_tuple();
  joined_tuple_.set_right(right_tuple_);
  return RC::SUCCESS;
}

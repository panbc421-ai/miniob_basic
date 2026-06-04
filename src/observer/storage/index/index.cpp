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
// Created by wangyunlai.wyl on 2021/5/19.
//

#include <cstring>

#include "storage/index/index.h"

RC Index::init(const IndexMeta &index_meta, const std::vector<const FieldMeta *> &field_metas)
{
  if (field_metas.empty()) {
    return RC::INVALID_ARGUMENT;
  }

  index_meta_ = index_meta;
  field_metas_.clear();
  key_length_ = 0;
  for (const FieldMeta *field_meta : field_metas) {
    if (field_meta == nullptr) {
      return RC::INVALID_ARGUMENT;
    }
    field_metas_.push_back(*field_meta);
    key_length_ += field_meta->len();
  }
  return RC::SUCCESS;
}

void Index::build_record_key(const char *record, char *key) const
{
  int offset = 0;
  for (const FieldMeta &field_meta : field_metas_) {
    memcpy(key + offset, record + field_meta.offset(), field_meta.len());
    offset += field_meta.len();
  }
}

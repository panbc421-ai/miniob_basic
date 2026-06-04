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
// Created by Wangyunlai.wyl on 2021/5/18.
//

#include <cstring>

#include "storage/index/index_meta.h"
#include "storage/field/field_meta.h"
#include "storage/table/table_meta.h"
#include "common/lang/string.h"
#include "common/log/log.h"
#include "json/json.h"

const static Json::StaticString FIELD_NAME("name");
const static Json::StaticString FIELD_FIELD_NAME("field_name");
const static Json::StaticString FIELD_UNIQUE("is_unique");

static void split_field_names(const std::string &field_str, std::vector<std::string> &names)
{
  names.clear();
  std::string token;
  for (char c : field_str) {
    if (c == ',') {
      if (!token.empty()) {
        names.push_back(token);
        token.clear();
      }
    } else {
      token.push_back(c);
    }
  }
  if (!token.empty()) {
    names.push_back(token);
  }
}

RC IndexMeta::init(const char *name, const FieldMeta &field, bool unique)
{
  return init(name, std::vector<const FieldMeta *>{&field}, unique);
}

RC IndexMeta::init(const char *name, const std::vector<const FieldMeta *> &fields, bool unique)
{
  if (common::is_blank(name)) {
    LOG_ERROR("Failed to init index, name is empty.");
    return RC::INVALID_ARGUMENT;
  }
  if (fields.empty()) {
    LOG_ERROR("Failed to init index, no fields.");
    return RC::INVALID_ARGUMENT;
  }

  name_ = name;
  field_names_.clear();
  field_.clear();
  for (size_t i = 0; i < fields.size(); i++) {
    if (fields[i] == nullptr) {
      LOG_ERROR("Failed to init index, null field meta.");
      return RC::INVALID_ARGUMENT;
    }
    field_names_.push_back(fields[i]->name());
    if (i > 0) {
      field_.append(",");
    }
    field_.append(fields[i]->name());
  }
  is_unique_ = unique;
  return RC::SUCCESS;
}

bool IndexMeta::contains_field(const char *field) const
{
  if (field == nullptr) {
    return false;
  }
  for (const std::string &name : field_names_) {
    if (0 == strcmp(name.c_str(), field)) {
      return true;
    }
  }
  return false;
}

void IndexMeta::to_json(Json::Value &json_value) const
{
  json_value[FIELD_NAME] = name_;
  json_value[FIELD_FIELD_NAME] = field_;
  if (is_unique_) {
    json_value[FIELD_UNIQUE] = is_unique_;
  }
}

RC IndexMeta::from_json(const TableMeta &table, const Json::Value &json_value, IndexMeta &index)
{
  const Json::Value &name_value = json_value[FIELD_NAME];
  const Json::Value &field_value = json_value[FIELD_FIELD_NAME];
  if (!name_value.isString()) {
    LOG_ERROR("Index name is not a string. json value=%s", name_value.toStyledString().c_str());
    return RC::INTERNAL;
  }

  if (!field_value.isString()) {
    LOG_ERROR("Field name of index [%s] is not a string. json value=%s",
        name_value.asCString(),
        field_value.toStyledString().c_str());
    return RC::INTERNAL;
  }

  std::vector<std::string> names;
  split_field_names(field_value.asString(), names);
  if (names.empty()) {
    LOG_ERROR("Deserialize index [%s]: empty field list", name_value.asCString());
    return RC::INTERNAL;
  }

  std::vector<const FieldMeta *> fields;
  fields.reserve(names.size());
  for (const std::string &name : names) {
    const FieldMeta *field = table.field(name.c_str());
    if (nullptr == field) {
      LOG_ERROR("Deserialize index [%s]: no such field: %s", name_value.asCString(), name.c_str());
      return RC::SCHEMA_FIELD_MISSING;
    }
    fields.push_back(field);
  }

  bool is_unique = false;
  const Json::Value &unique_value = json_value[FIELD_UNIQUE];
  if (unique_value.isBool()) {
    is_unique = unique_value.asBool();
  }

  return index.init(name_value.asCString(), fields, is_unique);
}

const char *IndexMeta::name() const
{
  return name_.c_str();
}

const char *IndexMeta::field() const
{
  return field_.c_str();
}

void IndexMeta::desc(std::ostream &os) const
{
  os << "index name=" << name_ << ", field=" << field_;
}

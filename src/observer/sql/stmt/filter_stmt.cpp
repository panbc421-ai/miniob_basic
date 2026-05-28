/* Copyright (c) 2021 OceanBase and/or its affiliates. All rights reserved.
miniob is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
         http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

#include "common/rc.h"
#include "common/log/log.h"
#include "common/lang/string.h"
#include "sql/stmt/filter_stmt.h"
#include "storage/db/db.h"
#include "storage/table/table.h"

FilterStmt::~FilterStmt()
{
  for (FilterUnit *unit : filter_units_) {
    delete unit;
  }
  filter_units_.clear();
}

RC FilterStmt::create(Db *db, Table *default_table, std::unordered_map<std::string, Table *> *tables,
    const ConditionSqlNode *conditions, int condition_num, FilterStmt *&stmt)
{
  RC rc = RC::SUCCESS;
  stmt = nullptr;

  FilterStmt *tmp_stmt = new FilterStmt();
  for (int i = 0; i < condition_num; i++) {
    FilterUnit *filter_unit = nullptr;
    rc = create_filter_unit(db, default_table, tables, conditions[i], filter_unit);
    if (rc != RC::SUCCESS) {
      delete tmp_stmt;
      LOG_WARN("failed to create filter unit. condition index=%d", i);
      return rc;
    }
    tmp_stmt->filter_units_.push_back(filter_unit);
  }

  stmt = tmp_stmt;
  return rc;
}

RC get_table_and_field(Db *db, Table *default_table, std::unordered_map<std::string, Table *> *tables,
    const RelAttrSqlNode &attr, Table *&table, const FieldMeta *&field)
{
  if (common::is_blank(attr.relation_name.c_str())) {
    table = default_table;
  } else if (nullptr != tables) {
    auto iter = tables->find(attr.relation_name);
    if (iter != tables->end()) {
      table = iter->second;
    }
  } else {
    table = db->find_table(attr.relation_name.c_str());
  }
  if (nullptr == table) {
    LOG_WARN("No such table: attr.relation_name: %s", attr.relation_name.c_str());
    return RC::SCHEMA_TABLE_NOT_EXIST;
  }

  field = table->table_meta().field(attr.attribute_name.c_str());
  if (nullptr == field) {
    LOG_WARN("no such field in table: table %s, field %s", table->name(), attr.attribute_name.c_str());
    table = nullptr;
    return RC::SCHEMA_FIELD_NOT_EXIST;
  }
  return RC::SUCCESS;
}

// 解析一个表达式侧（left 或 right），如果结果是简单类型则转换为 FilterObj
static RC resolve_side(Expression *&expr, Table *default_table,
                       std::unordered_map<std::string, Table *> *tables,
                       FilterObj &obj, bool &is_simple)
{
  if (expr == nullptr) return RC::SUCCESS;
  std::unique_ptr<Expression> e(expr);
  expr = nullptr; // 转移所有权
  RC rc = resolve_expression(e, default_table, tables);
  if (rc != RC::SUCCESS) return rc;
  if (e->type() == ExprType::FIELD) {
    obj.init_attr(static_cast<FieldExpr *>(e.get())->field());
    is_simple = true;
    return RC::SUCCESS;
  } else if (e->type() == ExprType::VALUE) {
    Value v;
    e->try_get_value(v);
    obj.init_value(v);
    is_simple = true;
    return RC::SUCCESS;
  }
  // 复杂表达式: 放回 expr 指针，稍后存入 FilterUnit
  is_simple = false;
  expr = e.release();
  return RC::SUCCESS;
}

RC FilterStmt::create_filter_unit(Db *db, Table *default_table, std::unordered_map<std::string, Table *> *tables,
    const ConditionSqlNode &condition, FilterUnit *&filter_unit)
{
  RC rc = RC::SUCCESS;

  CompOp comp = condition.comp;
  if (comp < EQUAL_TO || comp >= NO_OP) {
    LOG_WARN("invalid compare operator : %d", comp);
    return RC::INVALID_ARGUMENT;
  }

  // 检查是否有表达式操作数
  bool has_expr = (condition.left_expr != nullptr || condition.right_expr != nullptr);

  if (!has_expr) {
    // === 纯 legacy 路径 ===
    filter_unit = new FilterUnit;
    filter_unit->set_comp(comp);
    if (condition.left_is_attr) {
      Table *table = nullptr;
      const FieldMeta *field = nullptr;
      rc = get_table_and_field(db, default_table, tables, condition.left_attr, table, field);
      if (rc != RC::SUCCESS) { delete filter_unit; return rc; }
      FilterObj obj; obj.init_attr(Field(table, field));
      filter_unit->set_left(obj);
    } else {
      FilterObj obj; obj.init_value(condition.left_value);
      filter_unit->set_left(obj);
    }
    if (condition.right_is_attr) {
      Table *table = nullptr;
      const FieldMeta *field = nullptr;
      rc = get_table_and_field(db, default_table, tables, condition.right_attr, table, field);
      if (rc != RC::SUCCESS) { delete filter_unit; return rc; }
      FilterObj obj; obj.init_attr(Field(table, field));
      filter_unit->set_right(obj);
    } else {
      FilterObj obj; obj.init_value(condition.right_value);
      filter_unit->set_right(obj);
    }
  } else {
    // === 表达式路径：解析两边，尝试简化为 legacy ===
    FilterObj left_obj, right_obj;
    bool left_simple = false, right_simple = false;

    // 转移并解析 left
    Expression *left_e = condition.left_expr;
    if (left_e) const_cast<ConditionSqlNode &>(condition).left_expr = nullptr;
    rc = resolve_side(left_e, default_table, tables, left_obj, left_simple);
    if (rc != RC::SUCCESS) return rc;

    // 转移并解析 right
    Expression *right_e = condition.right_expr;
    if (right_e) const_cast<ConditionSqlNode &>(condition).right_expr = nullptr;
    rc = resolve_side(right_e, default_table, tables, right_obj, right_simple);
    if (rc != RC::SUCCESS) { delete left_e; return rc; }

    if (left_simple && right_simple) {
      // 两边都是简单类型: 使用 legacy 路径
      filter_unit = new FilterUnit();
      filter_unit->set_comp(comp);
      filter_unit->set_left(left_obj);
      filter_unit->set_right(right_obj);
    } else {
      // 复杂表达式: 使用表达式路径
      filter_unit = new FilterUnit();
      filter_unit->set_comp(comp);
      if (!left_simple && left_e) {
        filter_unit->set_left_expr(left_e);
      } else if (left_simple) {
        // 简单侧不用表达式，但 FilterUnit 需要知道这是 expr_based
        // 用 FieldExpr 或 ValueExpr 包装
        if (left_obj.is_attr) {
          filter_unit->set_left_expr(new FieldExpr(left_obj.field));
        } else {
          filter_unit->set_left_expr(new ValueExpr(left_obj.value));
        }
      }
      if (!right_simple && right_e) {
        filter_unit->set_right_expr(right_e);
      } else if (right_simple) {
        if (right_obj.is_attr) {
          filter_unit->set_right_expr(new FieldExpr(right_obj.field));
        } else {
          filter_unit->set_right_expr(new ValueExpr(right_obj.value));
        }
      }
    }
  }

  // Type validation (legacy path only)
  if (!filter_unit->is_expr_based()) {
    if (filter_unit->left().is_attr && !filter_unit->right().is_attr) {
      const FieldMeta *field = filter_unit->left().field.meta();
      if (field->type() == DATES && filter_unit->right().value.attr_type() == CHARS) {
        delete filter_unit; return RC::INVALID_ARGUMENT;
      }
    } else if (!filter_unit->left().is_attr && filter_unit->right().is_attr) {
      const FieldMeta *field = filter_unit->right().field.meta();
      if (field->type() == DATES && filter_unit->left().value.attr_type() == CHARS) {
        delete filter_unit; return RC::INVALID_ARGUMENT;
      }
    }
  }

  return rc;
}

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
// Created by Wangyunlai on 2022/07/05.
//

#include "sql/expr/expression.h"
#include "sql/expr/tuple.h"
#include "sql/parser/parse_defs.h"
#include "storage/table/table.h"
#include "storage/field/field_meta.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <unordered_map>

using namespace std;

thread_local const Tuple *g_outer_tuple = nullptr;

ColumnAliasMap build_column_alias_map(const SelectSqlNode &sel,
    unordered_map<string, Table *> &table_map, Table *default_table)
{
  ColumnAliasMap result;
  for (const SelectExprNode &item : sel.expressions) {
    if (item.alias.empty() || item.expr == nullptr) {
      continue;
    }
    if (item.expr->type() != ExprType::UNBOUND_FIELD) {
      continue;
    }
    auto *uf = static_cast<UnboundFieldExpr *>(item.expr);
    Table *table = default_table;
    const string &tn = uf->table_name();
    if (!tn.empty()) {
      auto it = table_map.find(tn);
      if (it != table_map.end()) {
        table = it->second;
      }
    }
    if (table == nullptr) {
      continue;
    }
    const FieldMeta *fm = table->table_meta().field(uf->field_name().c_str());
    if (fm == nullptr) {
      continue;
    }
    // Parser sets alias to field name when AS is omitted; only explicit aliases count.
    if (item.alias == uf->field_name()) {
      continue;
    }
    ColumnAliasBinding binding;
    binding.table_name = table->name();
    binding.field_name = uf->field_name();
    binding.field_meta = fm;
    result[item.alias] = binding;
  }
  return result;
}

static bool try_resolve_outer_field(const string &field_name,
    unordered_map<string, Table *> *outer_table_map, ColumnAliasMap *outer_column_aliases,
    unique_ptr<Expression> &expr)
{
  if (outer_column_aliases != nullptr) {
    auto alias_it = outer_column_aliases->find(field_name);
    if (alias_it != outer_column_aliases->end()) {
      auto *cf = new CorrelatedFieldExpr(alias_it->second.table_name, alias_it->second.field_name,
          alias_it->second.field_meta);
      cf->set_name(expr->name());
      expr.reset(cf);
      return true;
    }
  }
  if (outer_table_map != nullptr) {
    for (const auto &kv : *outer_table_map) {
      const FieldMeta *fm = kv.second->table_meta().field(field_name.c_str());
      if (fm != nullptr) {
        auto *cf = new CorrelatedFieldExpr(kv.second->name(), field_name, fm);
        cf->set_name(expr->name());
        expr.reset(cf);
        return true;
      }
    }
  }
  return false;
}

// 递归解析表达式树中的 UnboundFieldExpr → FieldExpr
RC resolve_expression(unique_ptr<Expression> &expr,
    Table *default_table,
    unordered_map<string, Table *> *table_map,
    unordered_map<string, Table *> *outer_table_map,
    ColumnAliasMap *outer_column_aliases)
{
  if (expr == nullptr) return RC::SUCCESS;

  switch (expr->type()) {
    case ExprType::UNBOUND_FIELD: {
      auto *unbound = static_cast<UnboundFieldExpr *>(expr.get());
      const string &table_name = unbound->table_name();
      const string &field_name = unbound->field_name();

      Table *table = nullptr;
      if (!table_name.empty() && table_map != nullptr) {
        auto it = table_map->find(table_name);
        if (it != table_map->end()) table = it->second;
      }
      // Fallback to outer table_map for correlated subqueries
      bool is_correlated = false;
      if (table == nullptr && !table_name.empty() && outer_table_map != nullptr) {
        auto it = outer_table_map->find(table_name);
        if (it != outer_table_map->end()) {
          table = it->second;
          is_correlated = true;
        }
      }
      if (table == nullptr) table = default_table;
      if (table == nullptr) {
        if (try_resolve_outer_field(field_name, outer_table_map, outer_column_aliases, expr)) {
          return RC::SUCCESS;
        }
        LOG_WARN("cannot resolve field %s.%s: no table",
                 table_name.c_str(), field_name.c_str());
        return RC::SCHEMA_FIELD_MISSING;
      }
      const FieldMeta *fm = table->table_meta().field(field_name.c_str());
      if (fm == nullptr) {
        if (try_resolve_outer_field(field_name, outer_table_map, outer_column_aliases, expr)) {
          return RC::SUCCESS;
        }
        LOG_WARN("no such field %s in table %s",
                 field_name.c_str(), table->name());
        return RC::SCHEMA_FIELD_MISSING;
      }
      if (is_correlated) {
        // Use real table name: outer RowTuple matches by table name, not alias
        auto *cf = new CorrelatedFieldExpr(table->name(), field_name, fm);
        cf->set_name(expr->name());
        expr.reset(cf);
      } else {
        auto *resolved = new FieldExpr(table, fm);
        resolved->set_name(expr->name());
        expr.reset(resolved);
      }
      return RC::SUCCESS;
    }
    case ExprType::ARITHMETIC: {
      auto *arith = static_cast<ArithmeticExpr *>(expr.get());
      RC rc = resolve_expression(arith->left(), default_table, table_map, outer_table_map, outer_column_aliases);
      if (rc != RC::SUCCESS) return rc;
      if (arith->right()) {
        rc = resolve_expression(arith->right(), default_table, table_map, outer_table_map, outer_column_aliases);
        if (rc != RC::SUCCESS) return rc;
      }
      return RC::SUCCESS;
    }
    case ExprType::COMPARISON: {
      auto *cmp = static_cast<ComparisonExpr *>(expr.get());
      RC rc = resolve_expression(cmp->left(), default_table, table_map, outer_table_map, outer_column_aliases);
      if (rc != RC::SUCCESS) return rc;
      return resolve_expression(cmp->right(), default_table, table_map, outer_table_map, outer_column_aliases);
    }
    case ExprType::CONJUNCTION: {
      auto *conj = static_cast<ConjunctionExpr *>(expr.get());
      for (auto &child : conj->children()) {
        RC rc = resolve_expression(child, default_table, table_map, outer_table_map, outer_column_aliases);
        if (rc != RC::SUCCESS) return rc;
      }
      return RC::SUCCESS;
    }
    case ExprType::CAST: {
      auto *cast = static_cast<CastExpr *>(expr.get());
      return resolve_expression(cast->child(), default_table, table_map, outer_table_map, outer_column_aliases);
    }
    case ExprType::AGGREGATION: {
      auto *agg = static_cast<AggregationExpr *>(expr.get());
      const std::string &table_name = agg->table_name();
      const std::string &field_name = agg->field_name();
      if (field_name == "*") {
        // COUNT(*) — leave as is, resolved later
        return RC::SUCCESS;
      }
      Table *table = nullptr;
      if (!table_name.empty() && table_map != nullptr) {
        auto it = table_map->find(table_name);
        if (it != table_map->end()) table = it->second;
      }
      if (table == nullptr) table = default_table;
      if (table == nullptr) {
        LOG_WARN("cannot resolve aggregation field %s.%s: no table",
                 table_name.c_str(), field_name.c_str());
        return RC::SCHEMA_FIELD_MISSING;
      }
      const FieldMeta *fm = table->table_meta().field(field_name.c_str());
      if (fm == nullptr) {
        LOG_WARN("no such field %s in table %s",
                 field_name.c_str(), table->name());
        return RC::SCHEMA_FIELD_MISSING;
      }
      // Replace AggregationExpr with FieldExpr
      auto *resolved = new FieldExpr(table, fm);
      resolved->set_name(expr->name());
      expr.reset(resolved);
      return RC::SUCCESS;
    }
    case ExprType::FUNCTION: {
      auto *fn = static_cast<FunctionExpr *>(expr.get());
      for (auto &arg : fn->args()) {
        RC rc = resolve_expression(arg, default_table, table_map, outer_table_map, outer_column_aliases);
        if (rc != RC::SUCCESS) return rc;
      }
      return RC::SUCCESS;
    }
    default:
      return RC::SUCCESS;
  }
}

static bool match_like(const char *str, int str_len, const char *pattern, int pat_len)
{
  int si = 0, pi = 0;
  int star_pi = -1;
  int match_si = 0;

  while (si < str_len) {
    if (pi < pat_len && pattern[pi] == '_') {
      si++;
      pi++;
    } else if (pi < pat_len && pattern[pi] == str[si]) {
      si++;
      pi++;
    } else if (pi < pat_len && pattern[pi] == '%') {
      star_pi = pi;
      match_si = si;
      pi++;
    } else if (star_pi != -1) {
      pi = star_pi + 1;
      match_si++;
      si = match_si;
    } else {
      return false;
    }
  }

  while (pi < pat_len && pattern[pi] == '%') {
    pi++;
  }

  return pi == pat_len;
}

RC FieldExpr::get_value(const Tuple &tuple, Value &value) const
{
  return tuple.find_cell(TupleCellSpec(table_name(), field_name()), value);
}

RC ValueExpr::get_value(const Tuple &tuple, Value &value) const
{
  value = value_;
  return RC::SUCCESS;
}

RC CorrelatedFieldExpr::get_value(const Tuple &tuple, Value &value) const
{
  if (g_outer_tuple == nullptr) {
    LOG_WARN("CorrelatedFieldExpr::get_value called without outer tuple context");
    return RC::INTERNAL;
  }
  RC rc = g_outer_tuple->find_cell(TupleCellSpec(table_name_.c_str(), field_name_.c_str()), value);
  if (rc == RC::SUCCESS) {
    return rc;
  }
  return g_outer_tuple->find_cell(TupleCellSpec(field_name_.c_str()), value);
}

/////////////////////////////////////////////////////////////////////////////////
AggregationExpr::AggregationExpr(AggregationType agg_type, const std::string &table_name, const std::string &field_name)
    : agg_type_(agg_type), table_name_(table_name), field_name_(field_name)
{
  if (field_name == "*") {
    set_name("count(*)");
  } else {
    set_name(field_name);
  }
}

RC AggregationExpr::get_value(const Tuple &tuple, Value &value) const
{
  // AggregationExpr must be resolved to a FieldExpr before evaluation
  LOG_WARN("AggregationExpr::get_value called before resolution");
  return RC::INTERNAL;
}

CastExpr::CastExpr(unique_ptr<Expression> child, AttrType cast_type)
    : child_(std::move(child)), cast_type_(cast_type)
{}

CastExpr::~CastExpr()
{}

RC CastExpr::cast(const Value &value, Value &cast_value) const
{
  RC rc = RC::SUCCESS;
  if (this->value_type() == value.attr_type()) {
    cast_value = value;
    return rc;
  }

  switch (cast_type_) {
    case BOOLEANS: {
      bool val = value.get_boolean();
      cast_value.set_boolean(val);
    } break;
    default: {
      rc = RC::INTERNAL;
      LOG_WARN("unsupported convert from type %d to %d", child_->value_type(), cast_type_);
    }
  }
  return rc;
}

RC CastExpr::get_value(const Tuple &tuple, Value &cell) const
{
  RC rc = child_->get_value(tuple, cell);
  if (rc != RC::SUCCESS) {
    return rc;
  }

  return cast(cell, cell);
}

RC CastExpr::try_get_value(Value &value) const
{
  RC rc = child_->try_get_value(value);
  if (rc != RC::SUCCESS) {
    return rc;
  }

  return cast(value, value);
}

////////////////////////////////////////////////////////////////////////////////

ComparisonExpr::ComparisonExpr(CompOp comp, unique_ptr<Expression> left, unique_ptr<Expression> right)
    : comp_(comp), left_(std::move(left)), right_(std::move(right))
{}

ComparisonExpr::~ComparisonExpr()
{}

static bool is_div_zero_sentinel(const Value &v)
{
  if (v.attr_type() == AttrType::INTS && v.get_int() == numeric_limits<int>::max())
    return true;
  if (v.attr_type() == AttrType::FLOATS) {
    float fv = v.get_float();
    float fmax = numeric_limits<float>::max();
    if (fv >= fmax * (1.0f - 1e-7f))
      return true;
  }
  return false;
}

RC ComparisonExpr::compare_value(const Value &left, const Value &right, bool &result) const
{
  RC rc = RC::SUCCESS;
  // IS NULL / IS NOT NULL: only depend on the left operand's null-ness.
  if (comp_ == IS_NULL) {
    result = left.is_null();
    return rc;
  }
  if (comp_ == IS_NOT_NULL) {
    result = !left.is_null();
    return rc;
  }
  // NULL semantics: any other comparison with NULL returns false (unknown)
  if (left.is_null() || right.is_null()) {
    result = false;
    return rc;
  }
  int cmp_result = left.compare(right);
  result = false;
  // Division-by-zero sentinel: always false (NULL semantics)
  if (is_div_zero_sentinel(left) || is_div_zero_sentinel(right)) {
    result = false;
    return rc;
  }
  switch (comp_) {
    case EQUAL_TO: {
      result = (0 == cmp_result);
    } break;
    case LESS_EQUAL: {
      result = (cmp_result <= 0);
    } break;
    case NOT_EQUAL: {
      result = (cmp_result != 0);
    } break;
    case LESS_THAN: {
      result = (cmp_result < 0);
    } break;
    case GREAT_EQUAL: {
      result = (cmp_result >= 0);
    } break;
    case GREAT_THAN: {
      result = (cmp_result > 0);
    } break;
    case LIKE_OP: {
      string left_str = left.to_string();
      string right_str = right.to_string();
      result = match_like(left_str.c_str(), (int)left_str.length(),
                          right_str.c_str(), (int)right_str.length());
    } break;
    case NOT_LIKE: {
      string left_str = left.to_string();
      string right_str = right.to_string();
      result = !match_like(left_str.c_str(), (int)left_str.length(),
                           right_str.c_str(), (int)right_str.length());
    } break;
    default: {
      LOG_WARN("unsupported comparison. %d", comp_);
      rc = RC::INTERNAL;
    } break;
  }

  return rc;
}

RC ComparisonExpr::try_get_value(Value &cell) const
{
  if (left_->type() == ExprType::VALUE && right_->type() == ExprType::VALUE) {
    ValueExpr *left_value_expr = static_cast<ValueExpr *>(left_.get());
    ValueExpr *right_value_expr = static_cast<ValueExpr *>(right_.get());
    const Value &left_cell = left_value_expr->get_value();
    const Value &right_cell = right_value_expr->get_value();

    bool value = false;
    RC rc = compare_value(left_cell, right_cell, value);
    if (rc != RC::SUCCESS) {
      LOG_WARN("failed to compare tuple cells. rc=%s", strrc(rc));
    } else {
      cell.set_boolean(value);
    }
    return rc;
  }

  return RC::INVALID_ARGUMENT;
}

RC ComparisonExpr::get_value(const Tuple &tuple, Value &value) const
{
  Value left_value;
  Value right_value;

  RC rc = left_->get_value(tuple, left_value);
  if (rc != RC::SUCCESS) {
    LOG_WARN("failed to get value of left expression. rc=%s", strrc(rc));
    return rc;
  }
  rc = right_->get_value(tuple, right_value);
  if (rc != RC::SUCCESS) {
    LOG_WARN("failed to get value of right expression. rc=%s", strrc(rc));
    return rc;
  }

  bool bool_value = false;
  rc = compare_value(left_value, right_value, bool_value);
  if (rc == RC::SUCCESS) {
    value.set_boolean(bool_value);
  }
  return rc;
}

////////////////////////////////////////////////////////////////////////////////
ConjunctionExpr::ConjunctionExpr(Type type, vector<unique_ptr<Expression>> &children)
    : conjunction_type_(type), children_(std::move(children))
{}

RC ConjunctionExpr::get_value(const Tuple &tuple, Value &value) const
{
  RC rc = RC::SUCCESS;
  if (children_.empty()) {
    value.set_boolean(true);
    return rc;
  }

  Value tmp_value;
  for (const unique_ptr<Expression> &expr : children_) {
    rc = expr->get_value(tuple, tmp_value);
    if (rc != RC::SUCCESS) {
      LOG_WARN("failed to get value by child expression. rc=%s", strrc(rc));
      return rc;
    }
    bool bool_value = tmp_value.get_boolean();
    if ((conjunction_type_ == Type::AND && !bool_value) || (conjunction_type_ == Type::OR && bool_value)) {
      value.set_boolean(bool_value);
      return rc;
    }
  }

  bool default_value = (conjunction_type_ == Type::AND);
  value.set_boolean(default_value);
  return rc;
}

////////////////////////////////////////////////////////////////////////////////

ArithmeticExpr::ArithmeticExpr(ArithmeticExpr::Type type, Expression *left, Expression *right)
    : arithmetic_type_(type), left_(left), right_(right)
{}
ArithmeticExpr::ArithmeticExpr(ArithmeticExpr::Type type, unique_ptr<Expression> left, unique_ptr<Expression> right)
    : arithmetic_type_(type), left_(std::move(left)), right_(std::move(right))
{}

AttrType ArithmeticExpr::value_type() const
{
  if (!right_) {
    return left_->value_type();
  }

  if (left_->value_type() == AttrType::INTS &&
      right_->value_type() == AttrType::INTS &&
      arithmetic_type_ != Type::DIV) {
    return AttrType::INTS;
  }
  
  return AttrType::FLOATS;
}

RC ArithmeticExpr::calc_value(const Value &left_value, const Value &right_value, Value &value) const
{
  RC rc = RC::SUCCESS;

  const AttrType target_type = value_type();

  switch (arithmetic_type_) {
    case Type::ADD: {
      if (target_type == AttrType::INTS) {
        value.set_int(left_value.get_int() + right_value.get_int());
      } else {
        value.set_float(left_value.get_float() + right_value.get_float());
      }
    } break;

    case Type::SUB: {
      if (target_type == AttrType::INTS) {
        value.set_int(left_value.get_int() - right_value.get_int());
      } else {
        value.set_float(left_value.get_float() - right_value.get_float());
      }
    } break;

    case Type::MUL: {
      if (target_type == AttrType::INTS) {
        value.set_int(left_value.get_int() * right_value.get_int());
      } else {
        double result = static_cast<double>(left_value.get_float()) * static_cast<double>(right_value.get_float());
        if (!std::isfinite(result)) {
          value.set_null(true);
        } else {
          value.set_float(static_cast<float>(result));
        }
      }
    } break;

    case Type::DIV: {
      if (left_value.is_null() || right_value.is_null()) {
        value.set_null(true);
      } else if (target_type == AttrType::INTS) {
        if (right_value.get_int() == 0) {
          value.set_null(true);
        } else {
          value.set_int(left_value.get_int() / right_value.get_int());
        }
      } else {
        double divisor = static_cast<double>(right_value.get_float());
        if (std::fabs(divisor) < static_cast<double>(EPSILON)) {
          value.set_null(true);
        } else {
          double result = static_cast<double>(left_value.get_float()) / divisor;
          if (!std::isfinite(result)) {
            value.set_null(true);
          } else {
            value.set_float(static_cast<float>(result));
          }
        }
      }
    } break;

    case Type::NEGATIVE: {
      if (target_type == AttrType::INTS) {
        value.set_int(-left_value.get_int());
      } else {
        value.set_float(-left_value.get_float());
      }
    } break;

    default: {
      rc = RC::INTERNAL;
      LOG_WARN("unsupported arithmetic type. %d", arithmetic_type_);
    } break;
  }
  return rc;
}

RC ArithmeticExpr::get_value(const Tuple &tuple, Value &value) const
{
  RC rc = RC::SUCCESS;

  Value left_value;
  Value right_value;

  rc = left_->get_value(tuple, left_value);
  if (rc != RC::SUCCESS) {
    LOG_WARN("failed to get value of left expression. rc=%s", strrc(rc));
    return rc;
  }
  if (right_) {
    rc = right_->get_value(tuple, right_value);
    if (rc != RC::SUCCESS) {
      LOG_WARN("failed to get value of right expression. rc=%s", strrc(rc));
      return rc;
    }
  }
  return calc_value(left_value, right_value, value);
}

RC ArithmeticExpr::try_get_value(Value &value) const
{
  RC rc = RC::SUCCESS;

  Value left_value;
  Value right_value;

  rc = left_->try_get_value(left_value);
  if (rc != RC::SUCCESS) {
    LOG_WARN("failed to get value of left expression. rc=%s", strrc(rc));
    return rc;
  }

  if (right_) {
    rc = right_->try_get_value(right_value);
    if (rc != RC::SUCCESS) {
      LOG_WARN("failed to get value of right expression. rc=%s", strrc(rc));
      return rc;
    }
  }

  return calc_value(left_value, right_value, value);
}

////////////////////////////////////////////////////////////////////////////////

static int trimmed_char_length(const Value &v)
{
  std::string s = v.get_string();
  while (!s.empty() && s.back() == ' ') {
    s.pop_back();
  }
  return static_cast<int>(s.size());
}

static RC eval_function(const std::string &func_name, const std::vector<Value> &arg_values, Value &value)
{
  std::string fn = func_name;
  std::transform(fn.begin(), fn.end(), fn.begin(), ::tolower);

  if (fn == "length") {
    if (arg_values.size() != 1) return RC::INVALID_ARGUMENT;
    if (arg_values[0].attr_type() != CHARS && arg_values[0].attr_type() != TEXTS) {
      return RC::INVALID_ARGUMENT;
    }
    value.set_int(trimmed_char_length(arg_values[0]));
    return RC::SUCCESS;
  }
  if (fn == "round") {
    if (arg_values.size() < 1 || arg_values.size() > 2) return RC::INVALID_ARGUMENT;
    if (arg_values[0].attr_type() != FLOATS) return RC::INVALID_ARGUMENT;
    float f = arg_values[0].get_float();
    if (arg_values.size() == 1) {
      value.set_int(static_cast<int>(f > 0 ? f + 0.5f : f - 0.5f));
      return RC::SUCCESS;
    }
    if (arg_values[1].attr_type() != INTS) return RC::INVALID_ARGUMENT;
    int precision = arg_values[1].get_int();
    float factor = 1.0f;
    for (int i = 0; i < precision; i++) {
      factor *= 10.0f;
    }
    value.set_float(std::round(f * factor) / factor);
    return RC::SUCCESS;
  }
  if (fn == "date_format") {
    if (arg_values.size() < 1 || arg_values.size() > 2) return RC::INVALID_ARGUMENT;
    if (arg_values[0].attr_type() != DATES) return RC::INVALID_ARGUMENT;

    int date_val = arg_values[0].get_date();
    int y = date_val / 10000;
    int m = (date_val % 10000) / 100;
    int d = date_val % 100;

    std::string fmt = "%Y-%m-%d";
    if (arg_values.size() == 2) {
      if (arg_values[1].attr_type() != CHARS) return RC::INVALID_ARGUMENT;
      fmt = arg_values[1].get_string();
    }

    size_t pos;
    char buf[16];
    while ((pos = fmt.find("%Y")) != std::string::npos) {
      snprintf(buf, sizeof(buf), "%04d", y);
      fmt.replace(pos, 2, buf);
    }
    while ((pos = fmt.find("%m")) != std::string::npos) {
      snprintf(buf, sizeof(buf), "%02d", m);
      fmt.replace(pos, 2, buf);
    }
    while ((pos = fmt.find("%D")) != std::string::npos) {
      snprintf(buf, sizeof(buf), "%02d", d);
      fmt.replace(pos, 2, buf);
    }
    while ((pos = fmt.find("%d")) != std::string::npos) {
      snprintf(buf, sizeof(buf), "%02d", d);
      fmt.replace(pos, 2, buf);
    }
    value.set_string(fmt.c_str());
    return RC::SUCCESS;
  }

  return RC::UNIMPLENMENT;
}

AttrType FunctionExpr::value_type() const
{
  std::string fn = func_name_;
  std::transform(fn.begin(), fn.end(), fn.begin(), ::tolower);
  if (fn == "length") return INTS;
  if (fn == "round") return args_.size() >= 2 ? FLOATS : INTS;
  if (fn == "date_format") return CHARS;
  return UNDEFINED;
}

RC FunctionExpr::try_get_value(Value &value) const
{
  std::vector<Value> arg_values;
  arg_values.reserve(args_.size());
  for (auto &arg : args_) {
    Value v;
    RC rc = arg->try_get_value(v);
    if (rc != RC::SUCCESS) {
      return rc;
    }
    arg_values.push_back(v);
  }
  return eval_function(func_name_, arg_values, value);
}

RC FunctionExpr::get_value(const Tuple &tuple, Value &value) const
{
  std::vector<Value> arg_values;
  arg_values.reserve(args_.size());
  for (auto &arg : args_) {
    Value v;
    RC rc = arg->get_value(tuple, v);
    if (rc != RC::SUCCESS) return rc;
    arg_values.push_back(v);
  }
  return eval_function(func_name_, arg_values, value);
}

////////////////////////////////////////////////////////////////////////////////

RC InListExpr::get_value(const Tuple &tuple, Value &value) const
{
  (void)tuple;
  (void)value;
  LOG_WARN("InListExpr::get_value called — should have been resolved during filter creation");
  return RC::INTERNAL;
}

SubQueryExpr::SubQueryExpr(SelectSqlNode *select_node)
    : select_node_(select_node)
{
}

SubQueryExpr::~SubQueryExpr()
{
}

RC SubQueryExpr::get_value(const Tuple &tuple, Value &value) const
{
  // SubQueryExpr should be resolved (replaced) during filter creation.
  // If get_value is called, it means the subquery was not properly handled.
  LOG_WARN("SubQueryExpr::get_value called — should have been resolved during filter creation");
  return RC::INTERNAL;
}
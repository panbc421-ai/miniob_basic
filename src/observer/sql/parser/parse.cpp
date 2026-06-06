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
// Created by Meiyi
//

#include <mutex>
#include <string>
#include <vector>
#include <cstring>
#include <cctype>
#include <memory>
#include "sql/parser/parse.h"
#include "common/log/log.h"
#include "sql/expr/expression.h"
RC parse(char *st, ParsedSqlNode *sqln);

// Split multi-tuple INSERT like VALUES (a,b),(c,d) into individual INSERTs
static bool split_multi_insert(const char *sql, std::vector<std::string> &out)
{
  const char *p = sql;
  while (*p && isspace((unsigned char)*p)) p++;
  if (strncasecmp(p, "insert", 6) != 0) return false;

  const char *values = strcasestr(p, "values");
  if (!values) return false;
  values += 6;
  while (*values && isspace((unsigned char)*values)) values++;
  if (*values != '(') return false;

  // Find all parenthesized value tuples
  std::string prefix(sql, values - sql);
  const char *start = values;
  int depth = 0;
  while (*start) {
    if (*start == '(') {
      const char *tuple_start = start;
      depth = 1;
      start++;
      while (*start && depth > 0) {
        if (*start == '(') depth++;
        else if (*start == ')') depth--;
        start++;
      }
      if (depth != 0) return false;
      std::string tuple_sql = prefix + std::string(tuple_start, start - tuple_start);
      out.push_back(tuple_sql);
      while (*start && (isspace((unsigned char)*start) || *start == ',')) start++;
    } else {
      start++;
    }
  }
  return out.size() > 1;
}

int sql_parse(const char *st, ParsedSqlResult *sql_result);

static bool is_ident_char(char c)
{
  return std::isalnum(static_cast<unsigned char>(c)) || c == '_';
}

static bool keyword_at(const std::string &sql, size_t pos, const char *keyword)
{
  const size_t len = strlen(keyword);
  if (pos + len > sql.size()) {
    return false;
  }
  if (pos > 0 && is_ident_char(sql[pos - 1])) {
    return false;
  }
  if (pos + len < sql.size() && is_ident_char(sql[pos + len])) {
    return false;
  }
  return strncasecmp(sql.c_str() + pos, keyword, len) == 0;
}

static bool scan_sql_context(char c, char &quote, int &depth)
{
  if (quote != 0) {
    if (c == quote) {
      quote = 0;
    }
    return false;
  }
  if (c == '\'' || c == '"') {
    quote = c;
    return false;
  }
  if (c == '(') {
    depth++;
    return false;
  }
  if (c == ')' && depth > 0) {
    depth--;
    return false;
  }
  return depth == 0;
}

static size_t find_top_level_keyword(const std::string &sql, const char *keyword, size_t start = 0)
{
  char quote = 0;
  int depth = 0;
  for (size_t i = start; i < sql.size(); i++) {
    if (!scan_sql_context(sql[i], quote, depth)) {
      continue;
    }
    if (keyword_at(sql, i, keyword)) {
      return i;
    }
  }
  return std::string::npos;
}

static size_t find_where_clause_end(const std::string &sql, size_t where_body_start)
{
  size_t end = sql.size();
  for (const char *kw : {"group", "having", "order"}) {
    size_t pos = find_top_level_keyword(sql, kw, where_body_start);
    if (pos != std::string::npos && pos < end) {
      end = pos;
    }
  }
  if (end > 0 && sql[end - 1] == ';') {
    end--;
  }
  return end;
}

static std::vector<std::string> split_top_level_or(const std::string &where_body)
{
  std::vector<std::string> parts;
  char quote = 0;
  int depth = 0;
  size_t last = 0;
  for (size_t i = 0; i < where_body.size(); i++) {
    if (!scan_sql_context(where_body[i], quote, depth)) {
      continue;
    }
    if (keyword_at(where_body, i, "or")) {
      parts.emplace_back(where_body.substr(last, i - last));
      i += 1;
      last = i + 1;
    }
  }
  if (!parts.empty()) {
    parts.emplace_back(where_body.substr(last));
  }
  return parts;
}

static std::string trim_copy(const std::string &s)
{
  size_t begin = 0;
  while (begin < s.size() && std::isspace(static_cast<unsigned char>(s[begin]))) {
    begin++;
  }
  size_t end = s.size();
  while (end > begin && std::isspace(static_cast<unsigned char>(s[end - 1]))) {
    end--;
  }
  return s.substr(begin, end - begin);
}

static std::unique_ptr<Expression> condition_to_expression(ConditionSqlNode &cond)
{
  if (cond.left_expr == nullptr || cond.right_expr == nullptr) {
    return nullptr;
  }
  std::unique_ptr<Expression> left(cond.left_expr);
  std::unique_ptr<Expression> right(cond.right_expr);
  cond.left_expr = nullptr;
  cond.right_expr = nullptr;
  return std::unique_ptr<Expression>(new ComparisonExpr(cond.comp, std::move(left), std::move(right)));
}

static std::unique_ptr<Expression> conditions_to_expression(std::vector<ConditionSqlNode> &conditions)
{
  std::vector<std::unique_ptr<Expression>> children;
  for (ConditionSqlNode &cond : conditions) {
    std::unique_ptr<Expression> expr = condition_to_expression(cond);
    if (expr == nullptr) {
      return nullptr;
    }
    children.emplace_back(std::move(expr));
  }
  if (children.empty()) {
    return nullptr;
  }
  if (children.size() == 1) {
    return std::move(children.front());
  }
  return std::unique_ptr<Expression>(new ConjunctionExpr(ConjunctionExpr::Type::AND, children));
}

static bool try_parse_select_with_or(const char *sql, ParsedSqlResult *sql_result)
{
  std::string original(sql);
  size_t where_pos = find_top_level_keyword(original, "where");
  if (where_pos == std::string::npos) {
    return false;
  }

  size_t where_body_start = where_pos + strlen("where");
  size_t where_end = find_where_clause_end(original, where_body_start);
  std::string where_body = original.substr(where_body_start, where_end - where_body_start);
  std::vector<std::string> or_parts = split_top_level_or(where_body);
  if (or_parts.size() <= 1) {
    return false;
  }

  std::vector<std::unique_ptr<Expression>> or_children;
  ParsedSqlResult first_result;
  for (size_t i = 0; i < or_parts.size(); i++) {
    std::string branch = trim_copy(or_parts[i]);
    if (branch.empty()) {
      return false;
    }
    std::string branch_sql = original.substr(0, where_body_start) + " " + branch + original.substr(where_end);
    ParsedSqlResult branch_result;
    sql_parse(branch_sql.c_str(), &branch_result);
    if (branch_result.sql_nodes().empty() || branch_result.sql_nodes().front()->flag != SCF_SELECT) {
      return false;
    }
    std::unique_ptr<Expression> branch_expr =
        conditions_to_expression(branch_result.sql_nodes().front()->selection.conditions);
    if (branch_expr == nullptr) {
      return false;
    }
    or_children.emplace_back(std::move(branch_expr));
    if (i == 0) {
      first_result = std::move(branch_result);
    }
  }

  Value true_value(true);
  ConditionSqlNode combined;
  combined.left_expr = new ConjunctionExpr(ConjunctionExpr::Type::OR, or_children);
  combined.right_expr = new ValueExpr(true_value);
  combined.comp = EQUAL_TO;

  auto &base_node = first_result.sql_nodes().front();
  base_node->selection.conditions.clear();
  base_node->selection.conditions.emplace_back(std::move(combined));

  sql_result->add_sql_node(std::move(base_node));
  return true;
}

// 无 FROM 的纯标量 SELECT（如 select length('a') len1, length('b') len2）
// 直接改写为 CALC，走最短执行路径，避免 SelectStmt/FilterStmt 中间环节。
static void try_convert_scalar_select_to_calc(ParsedSqlResult *sql_result)
{
  if (sql_result == nullptr) {
    return;
  }
  for (auto &node_ptr : sql_result->sql_nodes()) {
    ParsedSqlNode &node = *node_ptr;
    if (node.flag != SCF_SELECT) {
      continue;
    }
    SelectSqlNode &sel = node.selection;
    if (!sel.relations.empty() || !sel.attributes.empty() || !sel.conditions.empty() ||
        !sel.group_by.empty() || !sel.order_by.empty() || sel.expressions.empty()) {
      continue;
    }
    bool has_agg = false;
    for (const SelectExprNode &se : sel.expressions) {
      if (se.agg_type != AGG_NONE) {
        has_agg = true;
        break;
      }
    }
    if (has_agg) {
      continue;
    }

    node.flag = SCF_CALC;
    for (SelectExprNode &se : sel.expressions) {
      if (se.expr != nullptr) {
        if (!se.alias.empty()) {
          se.expr->set_name(se.alias);
        }
        node.calc.expressions.push_back(se.expr);
        se.expr = nullptr;
      }
    }
    sel.expressions.clear();
  }
}

RC parse(const char *st, ParsedSqlResult *sql_result)
{
  if (try_parse_select_with_or(st, sql_result)) {
    try_convert_scalar_select_to_calc(sql_result);
    return RC::SUCCESS;
  }

  std::vector<std::string> inserts;
  if (split_multi_insert(st, inserts)) {
    // Multi-tuple INSERT: parse first tuple normally, then merge
    // values from remaining tuples into the same INSERT node.
    sql_parse(inserts[0].c_str(), sql_result);
    for (size_t i = 1; i < inserts.size(); i++) {
      ParsedSqlResult single_result;
      sql_parse(inserts[i].c_str(), &single_result);
      if (!single_result.sql_nodes().empty()) {
        auto &node = *single_result.sql_nodes().front();
        if (node.flag == SCF_INSERT) {
          for (auto &v : node.insertion.values) {
            sql_result->sql_nodes().front()->insertion.values.emplace_back(std::move(v));
          }
        }
      }
    }
    try_convert_scalar_select_to_calc(sql_result);
    return RC::SUCCESS;
  }
  sql_parse(st, sql_result);
  try_convert_scalar_select_to_calc(sql_result);
  return RC::SUCCESS;
}

CalcSqlNode::~CalcSqlNode()
{
  for (Expression *expr : expressions) {
    delete expr;
  }
  expressions.clear();
}

ConditionSqlNode::~ConditionSqlNode()
{
  delete left_expr;
  delete right_expr;
  left_expr = nullptr;
  right_expr = nullptr;
}

ConditionSqlNode &ConditionSqlNode::operator=(ConditionSqlNode &&other) noexcept
{
  if (this != &other) {
    delete left_expr; delete right_expr;
    left_is_attr = other.left_is_attr; left_value = std::move(other.left_value);
    left_attr = std::move(other.left_attr); comp = other.comp;
    right_is_attr = other.right_is_attr; right_attr = std::move(other.right_attr);
    right_value = std::move(other.right_value);
    left_expr = other.left_expr; right_expr = other.right_expr;
    other.left_expr = nullptr; other.right_expr = nullptr;
  }
  return *this;
}

ParsedSqlNode::ParsedSqlNode() : flag(SCF_ERROR)
{}

ParsedSqlNode::ParsedSqlNode(SqlCommandFlag _flag) : flag(_flag)
{}

void ParsedSqlResult::add_sql_node(std::unique_ptr<ParsedSqlNode> sql_node)
{
  sql_nodes_.emplace_back(std::move(sql_node));
}

////////////////////////////////////////////////////////////////////////////////

int sql_parse(const char *st, ParsedSqlResult *sql_result);

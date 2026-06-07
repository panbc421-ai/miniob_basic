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
#include "sql/parser/parse.h"
#include "common/log/log.h"
#include "sql/expr/expression.h"
RC parse(char *st, ParsedSqlNode *sqln);
int sql_parse(const char *st, ParsedSqlResult *sql_result);
static bool identifier_char(char c);

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

static const char *skip_spaces(const char *p)
{
  while (*p && isspace((unsigned char)*p)) {
    p++;
  }
  return p;
}

static bool parse_load_data_statement(const char *sql, ParsedSqlResult *sql_result)
{
  const char *p = skip_spaces(sql);
  if (strncasecmp(p, "load", 4) != 0) {
    return false;
  }
  p = skip_spaces(p + 4);
  if (strncasecmp(p, "data", 4) != 0) {
    return false;
  }
  p = skip_spaces(p + 4);
  if (strncasecmp(p, "infile", 6) != 0) {
    return false;
  }
  p = skip_spaces(p + 6);

  if (*p != '\'' && *p != '"') {
    return false;
  }
  const char quote = *p++;
  const char *file_start = p;
  while (*p && *p != quote) {
    p++;
  }
  if (*p != quote) {
    return false;
  }
  std::string file_name(file_start, p - file_start);
  p = skip_spaces(p + 1);

  if (strncasecmp(p, "into", 4) != 0) {
    return false;
  }
  p = skip_spaces(p + 4);
  if (strncasecmp(p, "table", 5) != 0) {
    return false;
  }
  p = skip_spaces(p + 5);

  const char *table_start = p;
  while (*p && identifier_char(*p)) {
    p++;
  }
  if (p == table_start) {
    return false;
  }

  std::unique_ptr<ParsedSqlNode> node(new ParsedSqlNode(SCF_LOAD_DATA));
  node->load_data.file_name = file_name;
  node->load_data.relation_name.assign(table_start, p - table_start);
  sql_result->add_sql_node(std::move(node));
  return true;
}

static std::vector<std::string> split_identifier_list(const std::string &text)
{
  std::vector<std::string> names;
  size_t start = 0;
  while (start < text.size()) {
    while (start < text.size() && isspace((unsigned char)text[start])) {
      start++;
    }
    size_t end = start;
    while (end < text.size() && text[end] != ',') {
      end++;
    }
    size_t trim_end = end;
    while (trim_end > start && isspace((unsigned char)text[trim_end - 1])) {
      trim_end--;
    }
    if (trim_end > start) {
      names.emplace_back(text.substr(start, trim_end - start));
    }
    start = end + 1;
  }
  return names;
}

static bool identifier_char(char c)
{
  return std::isalnum(static_cast<unsigned char>(c)) || c == '_';
}

static bool word_at(const std::string &text, size_t pos, const char *word)
{
  const size_t len = strlen(word);
  if (pos + len > text.size()) {
    return false;
  }
  if (pos > 0 && identifier_char(text[pos - 1])) {
    return false;
  }
  if (pos + len < text.size() && identifier_char(text[pos + len])) {
    return false;
  }
  return strncasecmp(text.c_str() + pos, word, len) == 0;
}

static std::string normalize_reserved_select_aliases(const char *select_sql,
    std::vector<std::pair<std::string, std::string>> &alias_rewrites)
{
  const std::string input(select_sql);
  std::string output;
  output.reserve(input.size());

  char quote = 0;
  for (size_t i = 0; i < input.size();) {
    char c = input[i];
    if (c == '\'' || c == '"') {
      if (quote == 0) {
        quote = c;
      } else if (quote == c) {
        quote = 0;
      }
      output.push_back(c);
      i++;
      continue;
    }

    if (quote == 0 && word_at(input, i, "as")) {
      const size_t as_start = i;
      size_t p = i + 2;
      while (p < input.size() && std::isspace(static_cast<unsigned char>(input[p]))) {
        p++;
      }

      if (word_at(input, p, "data")) {
        std::string temp_alias = "__miniob_alias_data_" + std::to_string(alias_rewrites.size());
        alias_rewrites.emplace_back(temp_alias, "data");
        output.append(input, as_start, p - as_start);
        output.append(temp_alias);
        i = p + 4;
        continue;
      }
    }

    output.push_back(c);
    i++;
  }
  return output;
}

static void restore_reserved_select_aliases(SelectSqlNode &select_sql,
    const std::vector<std::pair<std::string, std::string>> &alias_rewrites)
{
  if (alias_rewrites.empty()) {
    return;
  }

  for (SelectExprNode &expr : select_sql.expressions) {
    for (const auto &rewrite : alias_rewrites) {
      if (expr.alias == rewrite.first) {
        expr.alias = rewrite.second;
        if (expr.expr != nullptr) {
          expr.expr->set_name(rewrite.second);
        }
        break;
      }
    }
  }
}

// Support CREATE TABLE t(col defs...) SELECT ... without touching the generated
// parser tables. The table-definition part and the SELECT part are both parsed
// by the normal parser, then merged into one CREATE_TABLE node.
static bool parse_create_table_select(const char *sql, ParsedSqlResult *sql_result)
{
  const char *p = skip_spaces(sql);
  if (strncasecmp(p, "create", 6) != 0) {
    return false;
  }
  p = skip_spaces(p + 6);
  if (strncasecmp(p, "table", 5) != 0) {
    return false;
  }

  const char *open = strchr(p + 5, '(');
  if (open == nullptr) {
    return false;
  }

  int depth = 1;
  const char *close = open + 1;
  while (*close && depth > 0) {
    if (*close == '(') {
      depth++;
    } else if (*close == ')') {
      depth--;
    }
    close++;
  }
  if (depth != 0) {
    return false;
  }

  const char *select = skip_spaces(close);
  if (strncasecmp(select, "select", 6) != 0) {
    return false;
  }

  std::string create_sql(sql, close - sql);
  std::string select_sql(select);

  ParsedSqlResult create_result;
  ParsedSqlResult select_result;
  sql_parse(create_sql.c_str(), &create_result);
  sql_parse(select_sql.c_str(), &select_result);

  if (create_result.sql_nodes().size() != 1 || select_result.sql_nodes().size() != 1) {
    return false;
  }

  std::unique_ptr<ParsedSqlNode> &create_node = create_result.sql_nodes().front();
  std::unique_ptr<ParsedSqlNode> &select_node = select_result.sql_nodes().front();
  if (create_node->flag != SCF_CREATE_TABLE || select_node->flag != SCF_SELECT) {
    return false;
  }

  create_node->create_table.is_ctas = true;
  create_node->create_table.select_sql = new SelectSqlNode(std::move(select_node->selection));
  sql_result->add_sql_node(std::move(create_node));
  return true;
}

static bool parse_create_view_statement(const char *sql, ParsedSqlResult *sql_result)
{
  const char *p = skip_spaces(sql);
  if (strncasecmp(p, "create", 6) != 0) {
    return false;
  }
  p = skip_spaces(p + 6);
  if (strncasecmp(p, "view", 4) != 0) {
    return false;
  }
  p = skip_spaces(p + 4);

  const char *name_start = p;
  while (*p && !isspace((unsigned char)*p) && *p != '(') {
    p++;
  }
  if (p == name_start) {
    return false;
  }
  std::string view_name(name_start, p - name_start);

  p = skip_spaces(p);
  std::vector<std::string> column_names;
  if (*p == '(') {
    const char *open = p;
    int depth = 1;
    const char *close = open + 1;
    while (*close && depth > 0) {
      if (*close == '(') {
        depth++;
      } else if (*close == ')') {
        depth--;
      }
      close++;
    }
    if (depth != 0) {
      return false;
    }

    const char *column_start = open + 1;
    const char *column_end = close - 1;
    column_names = split_identifier_list(std::string(column_start, column_end - column_start));
    if (column_names.empty()) {
      return false;
    }
    p = skip_spaces(close);
  }

  if (strncasecmp(p, "as", 2) != 0) {
    return false;
  }
  p = skip_spaces(p + 2);
  if (strncasecmp(p, "select", 6) != 0) {
    return false;
  }

  std::vector<std::pair<std::string, std::string>> alias_rewrites;
  std::string normalized_select = normalize_reserved_select_aliases(p, alias_rewrites);

  ParsedSqlResult select_result;
  sql_parse(normalized_select.c_str(), &select_result);
  if (select_result.sql_nodes().size() != 1 || select_result.sql_nodes().front()->flag != SCF_SELECT) {
    return false;
  }

  std::unique_ptr<ParsedSqlNode> node(new ParsedSqlNode(SCF_CREATE_VIEW));
  node->create_view.view_name = view_name;
  node->create_view.column_names = std::move(column_names);
  node->create_view.select_sql = std::move(select_result.sql_nodes().front()->selection);
  restore_reserved_select_aliases(node->create_view.select_sql, alias_rewrites);
  sql_result->add_sql_node(std::move(node));
  return true;
}

int sql_parse(const char *st, ParsedSqlResult *sql_result);

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
  if (parse_load_data_statement(st, sql_result)) {
    return RC::SUCCESS;
  }
  if (parse_create_table_select(st, sql_result)) {
    return RC::SUCCESS;
  }
  if (parse_create_view_statement(st, sql_result)) {
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
    connect_or = other.connect_or;
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

#pragma once

#include <string>
#include <utility>
#include <vector>

#include "sql/parser/parse_defs.h"
#include "sql/stmt/stmt.h"

class Db;

class CreateViewStmt : public Stmt
{
public:
  CreateViewStmt(const std::string &view_name, SelectSqlNode &&select_sql)
      : view_name_(view_name), select_sql_(std::move(select_sql))
  {}
  CreateViewStmt(const std::string &view_name, const std::vector<std::string> &column_names,
      SelectSqlNode &&select_sql)
      : view_name_(view_name), column_names_(column_names), select_sql_(std::move(select_sql))
  {}
  ~CreateViewStmt() override = default;

  StmtType type() const override { return StmtType::CREATE_VIEW; }
  const std::string &view_name() const { return view_name_; }
  const std::vector<std::string> &column_names() const { return column_names_; }
  SelectSqlNode &select_sql() { return select_sql_; }

  static RC create(Db *db, CreateViewSqlNode &node, Stmt *&stmt);

private:
  std::string   view_name_;
  std::vector<std::string> column_names_;
  SelectSqlNode select_sql_;
};

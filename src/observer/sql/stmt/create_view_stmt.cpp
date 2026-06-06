#include "sql/stmt/create_view_stmt.h"

#include "event/sql_debug.h"

RC CreateViewStmt::create(Db *db, CreateViewSqlNode &node, Stmt *&stmt)
{
  (void)db;
  stmt = new CreateViewStmt(node.view_name, node.column_names, std::move(node.select_sql));
  sql_debug("create view statement: view name %s", node.view_name.c_str());
  return RC::SUCCESS;
}

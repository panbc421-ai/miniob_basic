#include "sql/stmt/create_table_stmt.h"
#include "event/sql_debug.h"

RC CreateTableStmt::create(Db *db, CreateTableSqlNode &create_table, Stmt *&stmt)
{
  (void)db;
  if (create_table.is_ctas && create_table.select_sql != nullptr) {
    stmt = new CreateTableStmt(create_table.relation_name, std::move(*create_table.select_sql));
    delete create_table.select_sql;
    create_table.select_sql = nullptr;
    sql_debug("create table as select: %s", create_table.relation_name.c_str());
    return RC::SUCCESS;
  }
  stmt = new CreateTableStmt(create_table.relation_name, create_table.attr_infos);
  sql_debug("create table statement: table name %s", create_table.relation_name.c_str());
  return RC::SUCCESS;
}

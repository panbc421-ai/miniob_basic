#pragma once

#include <vector>

#include "common/rc.h"
#include "sql/parser/parse_defs.h"

class Db;
class Trx;
struct SelectSqlNode;

RC materialize_select_as_table(Db *db, Trx *trx, bool auto_commit,
    const char *table_name, SelectSqlNode &select_sql,
    const std::vector<AttrInfoSqlNode> *specified_attrs = nullptr,
    const std::vector<std::string> *specified_column_names = nullptr,
    bool output_nullable = false);

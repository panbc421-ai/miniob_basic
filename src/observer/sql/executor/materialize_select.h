#pragma once

#include "common/rc.h"

class Db;
class Trx;
struct SelectSqlNode;

RC materialize_select_as_table(Db *db, Trx *trx, bool auto_commit,
    const char *table_name, SelectSqlNode &select_sql);

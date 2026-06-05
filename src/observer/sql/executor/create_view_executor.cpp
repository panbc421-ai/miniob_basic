#include "sql/executor/create_view_executor.h"
#include "sql/executor/materialize_select.h"
#include "sql/stmt/create_view_stmt.h"
#include "event/sql_event.h"
#include "event/session_event.h"
#include "session/session.h"
#include "storage/db/db.h"

RC CreateViewExecutor::execute(SQLStageEvent *sql_event)
{
  CreateViewStmt *stmt = static_cast<CreateViewStmt *>(sql_event->stmt());
  Session *session = sql_event->session_event()->session();
  Db *db = session->get_current_db();
  Trx *trx = session->current_trx();
  return materialize_select_as_table(db, trx, stmt->view_name().c_str(), stmt->select_sql());
}

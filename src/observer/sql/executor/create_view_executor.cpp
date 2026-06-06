#include "sql/executor/create_view_executor.h"

#include "event/session_event.h"
#include "event/sql_event.h"
#include "session/session.h"
#include "sql/executor/materialize_select.h"
#include "sql/stmt/create_view_stmt.h"
#include "storage/db/db.h"

RC CreateViewExecutor::execute(SQLStageEvent *sql_event)
{
  CreateViewStmt *stmt = static_cast<CreateViewStmt *>(sql_event->stmt());
  Session *session = sql_event->session_event()->session();
  const bool auto_commit = !session->is_trx_multi_operation_mode();
  return materialize_select_as_table(session->get_current_db(),
      session->current_trx(), auto_commit,
      stmt->view_name().c_str(), stmt->select_sql());
}

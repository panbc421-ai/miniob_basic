#pragma once
#include "common/rc.h"
class SQLStageEvent;

class UpdateExecutor
{
public:
  RC execute(SQLStageEvent *sql_event);
};
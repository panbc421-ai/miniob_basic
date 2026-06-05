#pragma once

#include "common/rc.h"

class SQLStageEvent;

class CreateViewExecutor
{
public:
  RC execute(SQLStageEvent *sql_event);
};

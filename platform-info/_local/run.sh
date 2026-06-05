#!/bin/bash
# Usage: run.sh <sql_file>
# Runs observer in CLI mode against the SQL file in a clean data dir,
# then prints only result lines (strips logs / prompts / NUL bytes).
set -u
ROOT=/mnt/c/Users/11463/Downloads/miniob-2023/miniob-2023-09fe80d885b23450fdb01d7b9276ad0f0b6151c0
SQL="${1:?need sql file}"
RUNDIR=$(mktemp -d /tmp/miniob_run.XXXXXX)
cd "$RUNDIR" || exit 1
"$ROOT/build_debug/bin/observer" -P cli -f "$ROOT/etc/observer.ini" < "$SQL" > "$RUNDIR/out.raw" 2>/tmp/miniob_err.log
EC=$?
# Strip NUL, strip leading "miniob > " prompts, drop known startup log lines.
tr -d '\000' < "$RUNDIR/out.raw" \
  | sed 's/^\(miniob > \)*//' \
  | grep -avE '^(Successfully|Not set log|Output config|Begin dump|Finish dump|\[\]|Extend one pool|buffer pool|open clog|All table|find [0-9]+ uncommitted|Default handler|\(Re\)Starting|The process Name|Successfully init_event|Configuration hasn|Failed to init|SedaConfig|Db has been|close clog|Successfully do cleanup|Shutdown|Successfully sync|$)'
echo "=== exit code: $EC ==="
cd /
rm -rf "$RUNDIR"

#!/bin/bash
L=/mnt/c/Users/11463/Downloads/miniob-2023/miniob-2023-09fe80d885b23450fdb01d7b9276ad0f0b6151c0/platform-info/_local
for f in "$@"; do
  echo "########## $f ##########"
  python3 "$L/sockrun.py" "$L/$f.sql"
  echo
done

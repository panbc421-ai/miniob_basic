#!/bin/bash
set -euo pipefail

export DEBIAN_FRONTEND=noninteractive
apt-get update -qq
apt-get install -y -qq build-essential cmake git python3 bison flex wget

TOPDIR="$(cd "$(dirname "$0")/.." && pwd)"
cd "$TOPDIR"
sed -i 's/\r$//' build.sh

DEPS="$TOPDIR/deps/3rd"
clone_if_needed() {
  local dir="$1" url="$2" branch="${3:-}"
  if [ ! -f "$dir/CMakeLists.txt" ]; then
    echo "cloning $url -> $dir"
    rm -rf "$dir"
    if [ -n "$branch" ]; then
      git clone --depth 1 -b "$branch" "$url" "$dir"
    else
      git clone --depth 1 "$url" "$dir"
    fi
  fi
}

clone_if_needed "$DEPS/libevent" https://github.com/libevent/libevent release-2.1.12-stable
clone_if_needed "$DEPS/jsoncpp" https://github.com/open-source-parsers/jsoncpp
clone_if_needed "$DEPS/googletest" https://github.com/google/googletest
clone_if_needed "$DEPS/benchmark" https://github.com/google/benchmark

# Docker on Windows uses NTFS bind mounts; CMake needs a native Linux filesystem
WORKDIR="/tmp/miniob_build"
rm -rf "$WORKDIR"
cp -a "$TOPDIR" "$WORKDIR"
cd "$WORKDIR"
find . -name '*.sh' -exec sed -i 's/\r$//' {} +

# Clean stale CMake caches on native Linux filesystem
for dep in libevent googletest benchmark jsoncpp; do
  rm -rf "deps/3rd/$dep/build"
done
rm -rf build build_debug build_release

echo "=== build.sh init ==="
bash build.sh init

echo "=== build observer ==="
bash build.sh debug -DENABLE_ASAN=OFF -DWITH_UNIT_TESTS=OFF --make -j2

echo "=== primary-unique test ==="
python3 test/case/miniob_test.py --test-cases=primary-unique \
  --compile-cmake-args="-DENABLE_ASAN=OFF -DWITH_UNIT_TESTS=OFF" \
  --compile-make-args="-j2" 2>&1 | tail -5

echo "=== composite unique manual test ==="
OBSERVER="$WORKDIR/build/bin/observer"
INI="$WORKDIR/etc/observer.ini"
TMPDIR="$(mktemp -d)"
cd "$TMPDIR"

run_sql() {
  printf '%s\n' "$@" | "$OBSERVER" -f "$INI" -P cli 2>/dev/null | grep -E 'SUCCESS|FAILURE|SQL_SYNTAX|DUPLICATE' || true
}

run_sql "CREATE TABLE unique_table2(id1 int, id2 int, col1 int, col2 int);"
run_sql "INSERT INTO unique_table2 VALUES (1,1,1,1);"
run_sql "INSERT INTO unique_table2 VALUES (2,1,1,1);"
echo -n "create composite index: "
run_sql "CREATE UNIQUE INDEX idx ON unique_table2(id1, id2);"
echo -n "insert (2,3,1,1): "
run_sql "INSERT INTO unique_table2 VALUES (2,3,1,1);"
echo -n "insert duplicate (2,1,1,1): "
run_sql "INSERT INTO unique_table2 VALUES (2,1,2,2);"

rm -rf "$TMPDIR"
echo "=== done ==="

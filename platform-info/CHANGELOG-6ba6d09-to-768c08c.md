# 本次修改文档

**基线 commit**: `6ba6d09` — fix: 复合索引仅前缀列等值才走 IndexScan，避免非前缀扫描崩溃  
**当前 HEAD**: `768c08c` — function: commit no-FROM scalar SELECT -> CALC rewrite (parse stage)  
**修改日期**: 2026-06-05  
**相对基线**: 5 个功能 commit + 平台信息/本地测试基础设施

---

## 1. 背景与目标

平台在 `6ba6d09` 得分 **160 / 500**（25 题中 10 题满分、15 题 0 分）。  
本轮在本地完成编译与 socket 协议测试闭环后，针对可明确复现的失败项做了修复。  
提测后 `768c08c` 得分 **240 / 500**（13 题满分），**已更新为当前基线**（见 `768c08c/summary.md`）。

相关文件：

| 文件 | 用途 |
|------|------|
| `platform-info/768c08c/summary.md` | **当前基线**平台详情（240/500） |
| `platform-info/6ba6d09/summary.md` | 历史基线（160/500） |
| `platform-info/fixes-log.md` | commit ↔ 题目 简表 |
| `platform-info/_local/` | 本地 SQL 用例 + 运行脚本 |

---

## 2. 本地环境与测试方式

### 2.1 编译环境

- **系统**: WSL Ubuntu 22.04，gcc 11.4
- **依赖**: `build.sh init` 安装至 `/usr/local`（libevent、jsoncpp、gtest、benchmark）
- **构建**: `bash build.sh debug --make -j4`
- **注意**: 不使用 Docker 镜像（避免磁盘占满）；语法改动后需 `src/observer/sql/parser/gen_parser.sh` 重新生成 yacc/lex

### 2.2 测试方式

| 脚本 | 说明 |
|------|------|
| `platform-info/_local/sockrun.py` | 启动 observer（unix socket + mvcc），按平台协议发 SQL（`\0` 结尾），读响应 |
| `platform-info/_local/sockall.sh` | 批量跑多个 `.sql` |
| `platform-info/_local/run.sh` | CLI 模式（调试用，非平台协议） |

示例：

```bash
# WSL 内
python3 platform-info/_local/sockrun.py platform-info/_local/null.sql
bash platform-info/_local/sockall.sh null orderby updateselect alias
```

---

## 3. Commit 清单（按时间顺序）

### 3.1 `68a7d3e` — null: IS NULL / IS NOT NULL

**对应题目**: `null`（平台：`9 is null` 误报 FAILURE，应返回空集）

**问题**: 词法/语法无 `IS` token，无法解析 `expr IS NULL` / `expr IS NOT NULL`。

**修改文件**:

| 文件 | 改动 |
|------|------|
| `src/observer/sql/parser/lex_sql.l` | 新增关键字 `IS` |
| `src/observer/sql/parser/yacc_sql.y` | `condition` 增加 `expression IS NULL_T` / `expression IS NOT NULL_T` |
| `src/observer/sql/parser/parse_defs.h` | `CompOp` 增加 `IS_NULL`、`IS_NOT_NULL` |
| `src/observer/sql/expr/expression.cpp` | `ComparisonExpr::compare_value` 在通用 NULL 短路之前处理 IS NULL 语义 |
| `src/observer/sql/parser/yacc_sql.cpp` 等 | bison/flex 重新生成 |

**行为**:

- `SELECT * FROM t WHERE 9 IS NULL` → 空结果（常量非 NULL）
- `SELECT * FROM t WHERE 9 IS NOT NULL` → 全部行
- `WHERE num IS NULL` / `IS NOT NULL` 正常
- `WHERE null=1`、`1<>null` 等仍为空（三值逻辑 unknown → false）
- 聚合 `count(birthday)`、`avg(num)` 等跳过 NULL

**本地验证**: `null.sql`、`null2.sql`、`regress.sql` ✅

---

### 3.2 `d364437` — order-by: NULL 排序

**对应题目**: `order-by`（平台：NULL 未排最前，顺序错乱）

**问题**: `SortPhysicalOperator` 比较逻辑未处理 `Value::is_null()`，`cmp` 恒为 0，NULL 位置随机。

**修改文件**:

| 文件 | 改动 |
|------|------|
| `src/observer/sql/operator/sort_physical_operator.cpp` | 排序比较前：NULL 视为最小；双 NULL 相等；ASC 时 NULL 在前 |

**本地验证**: `orderby.sql` → `NULL, NULL, 2, 2, 5, 6, 6, 22` ✅

---

### 3.3 `f1ebb06` — update-select: 多列 SET + 标量子查询

**对应题目**: `update-select`（平台：`UPDATE ... SET t_name=..., col2=... WHERE id=5` 误更新全表）

**问题**:

1. 语法仅支持 `UPDATE t SET col = val` 单列
2. Executor 只写一列；多列 SET 解析丢失第二列及以后

**修改文件**:

| 文件 | 改动 |
|------|------|
| `parse_defs.h` | 新增 `UpdateAssignmentNode`；`UpdateSqlNode` 改为 `assignments` 向量 |
| `yacc_sql.y` | `update_stmt` → `UPDATE ID SET update_assign update_assign_list where`；支持 `col = value` 与 `col = (select ...)` |
| `update_stmt.h/cpp` | `UpdateUnit` 列表；解析多赋值 |
| `update_executor.cpp` | 重写：子查询求值 → 读行 → `make_record` 重建 → delete + insert（NULL/char/索引一致） |

**本地验证** (`updateselect.sql`):

- `SET t_name='9UBT4J5', col2=2 WHERE id=5` → 仅 id=5 行变化 ✅
- 单列 `UPDATE ... WHERE id=2` 回归 ✅
- `SET col1=(select max(col1) from t2) WHERE id=1` → col1=7 ✅

---

### 3.4 `7fd131d` — alias: `rel.*` 星号展开

**对应题目**: `alias`（平台：含 `t2.*` 的 SELECT 整句 FAILURE）

**问题**: `select_expr_list` 无法解析 `t1.id, t2.*` 中的 `ID DOT '*'`。

**修改文件**:

| 文件 | 改动 |
|------|------|
| `parse_defs.h` | `SelectExprNode` 增加 `is_star`、`star_table` |
| `yacc_sql.y` | `select_expr_item` 增加 `\| ID DOT '*'` |
| `select_stmt.cpp` | 遇到 `is_star` 时按别名/表名展开 `wildcard_fields` |
| `filter_stmt.cpp` | 子查询 clone 时复制 star 字段 |

**本地验证** (`alias.sql`):

- 6 行数值与平台期望一致 ✅
- **待平台确认**: 表头是否要求 `num | age | ...` 别名；当前为 `table_alias_1.id | ...` 形式

---

### 3.5 `768c08c` — function: 无 FROM 标量 SELECT → CALC

**对应题目**: `function`（平台 0 分；本地已能输出 `36 | 12`）

**问题**: 工作区长期存在未提交的 `parse.cpp` 改写逻辑；若不上传，平台构建与本地验证不一致。

**修改文件**:

| 文件 | 改动 |
|------|------|
| `src/observer/sql/parser/parse.cpp` | `try_convert_scalar_select_to_calc()`：无 FROM、无 WHERE/GROUP/ORDER、无聚合的 `SELECT expr...` 在 parse 后改写为 `SCF_CALC`，保留列别名 |

**典型 SQL**:

```sql
select length('E2NO0VF4EHPV72MC73YT54585CDTYF22ZWVP') len1,
       length('W8AOL9JDY025') len2;
```

**本地验证**: `function.sql` → `36 | 12` ✅（socket 协议）

**备注**: 若平台此前因 OOM 未成功编译 `6ba6d09`，function 0 分可能为**构建问题**而非逻辑问题；本次 commit 确保该路径进入版本库。

---

## 4. 变更统计（相对 `6ba6d09`）

```
32 files changed, 1831 insertions(+), 927 deletions(-)
```

**业务代码**（不含生成器大块 diff 中的重复行）核心触及：

- Parser: `lex_sql.l`, `yacc_sql.y`, `parse_defs.h`, `parse.cpp`
- Stmt: `select_stmt.cpp`, `update_stmt.*`, `filter_stmt.cpp`
- Expr/算子: `expression.cpp`, `sort_physical_operator.cpp`, `update_executor.cpp`

**新增基础设施**:

- `platform-info/6ba6d09/summary.md` — 基线失败分析
- `platform-info/fixes-log.md` — 修复简表
- `platform-info/_local/*` — 本地用例与 `sockrun.py`

---

## 5. 平台实测结果（`768c08c`，已更新为基线）

**总分**：160 → **240**（+80）

| 题目 | 6ba6d09 | 768c08c | commit | 结论 |
|------|---------|---------|--------|------|
| order-by | 0 | **20** | `d364437` | ✅ 修复生效 |
| big-query | 0 | **30** | — | 非本轮明确修复 |
| big-write | 0 | **30** | — | 非本轮明确修复 |
| null | 0 | 0 | `68a7d3e` | ❌ 平台换考 `min(num)` |
| update-select | 0 | 0 | `f1ebb06` | ❌ 平台换考子查询 SET |
| alias | 0 | 0 | `7fd131d` | ❌ 平台换考 JOIN 聚合 |
| function | 0 | 0 | `768c08c` | ❌ 仍失败 |

**仍待修复**（12 题，260 分）：见 [`768c08c/summary.md`](768c08c/summary.md)。

---

## 6. 提测前检查清单

- [ ] 本地 `bash build.sh debug --make -j4` 无 error
- [ ] `git log 6ba6d09..HEAD` 为上述 5 个 commit
- [ ] 工作区无未提交的业务代码（`git status` 干净）
- [ ] 由你在本机执行 `git push`（本环境不上 GitHub）
- [ ] 平台冷却 600s 后提测
- [x] 提测后 detail 已记入 `platform-info/768c08c/summary.md`（当前基线）

---

## 7. Git 命令参考

```bash
# 查看本轮全部提交
git log --oneline 6ba6d09..HEAD

# 查看相对基线的文件列表
git diff --name-only 6ba6d09..HEAD

# 推送（需你在有 VPN/网络的环境执行）
git push origin main
```

---

## 8. 附录：本地用例与期望摘要

| 文件 | 关键断言 |
|------|----------|
| `null.sql` | `9 is null` 空集；`num is null` 仅 NULL 行 |
| `null2.sql` | `null=1` 空集；`count(birthday)=3`；`avg(num)=15` |
| `orderby.sql` | `order by id` 以 NULL 开头 |
| `updateselect.sql` | 多列 WHERE 只改 id=5；子查询 max→7 |
| `alias.sql` | 6 行 join 结果，非 FAILURE |
| `function.sql` | `36 \| 12` |
| `regress.sql` | where/like/group/agg/in/update 无回归 |

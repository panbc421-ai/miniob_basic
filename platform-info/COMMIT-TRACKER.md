# Commit 追踪与平台 Detail 分析

> **选定基线：`071909d`**（260/500）  
> 不采用 `9d6888d` / `4f47e69` 作为开发起点（见下文对比）。

---

## 1. 基线选择依据

| Commit | 总分 | 相对 071909d | 结论 |
|--------|------|--------------|------|
| **071909d** | **260** | — | **基线**：aggregation-func✓ group-by✓，无列序/AVG 回归 |
| 9d6888d | 260 | group-by 20→0（SUM 全 NULL） | 不合并 |
| 4f47e69 | ~230 | −aggregation-func −group-by，expression 列错位，CTAS/view 回归 | **禁止整包 cherry-pick** |

### 071909d 已通过（260 分）

aggregation-func, basic, big-order-by, big-query, big-write, date, drop-table, **group-by**, join-tables, like, order-by, simple-sub-query, update, update-mvcc

### 071909d 待修复（240 分）

| 题目 | 分 | 平台 Detail 要点 |
|------|-----|------------------|
| alias | 20 | `t1.id as num ... where id in (select ... where num > t2.id)` → FAILURE |
| complex-sub-query | 30 | OR+标量子查询缺行 `54\|31\|90.79` |
| create-table-select | 20 | CTAS 本地 SUCCESS，平台 FAILURE（需 parser+executor） |
| create-view | 20 | 同上 |
| expression | 20 | JOIN 多行 `6\|0.29` `9\|0.29`；9d6888d 另有除零溢出 |
| function | 20 | `length(...)` 期望 `32\|5` 等 |
| null | 20 | `UPDATE SET name=null WHERE num is null` → `NULL\|N8` 应为 `NULL\|NULL` |
| multi-index | 20 | DELETE 后仍见 `4\|2\|29.45` 等 |
| text | 20 | 超长 TEXT insert 崩溃 |
| unique | 20 | 合法 INSERT 判 FAILURE |
| update-select | 20 | `SET col2=(select ...)` FAILURE |

---

## 2. 三版 Detail 对比（用户反馈）

### aggregation-func

| 版本 | 现象 |
|------|------|
| 071909d / 9d6888d | 10 分通过 |
| 4f47e69 | `50.63` vs `50.62`（AVG 精度回归，find_cell 回退导致） |

### group-by

| 版本 | 现象 |
|------|------|
| 071909d | `having count(*)>1` 正常 |
| 9d6888d / 4f47e69 | SUM 全 NULL 或 HAVING 无输出 |

### expression

| 版本 | 现象 |
|------|------|
| 071909d | JOIN 过滤不严，多出行 |
| 9d6888d | 除零应 NULL，得 `3.4e38` 溢出 |
| 4f47e69 | **列顺序对调**（`111.19\|105.6\|8`） |

### create-view / CTAS

| 版本 | 现象 |
|------|------|
| 071909d | 未实现 → FAILURE |
| 4f47e69 | 本地 SUCCESS → 平台 FAILURE（列序改动连带） |

---

## 3. 本批实现（基于 071909d，工作区未提交）

### 批次 A — 子查询 / 过滤

| 改动文件 | 题目 | 说明 |
|----------|------|------|
| `filter_stmt.cpp` | alias, update-select, null, complex-sub-query | `FilterStmt::eval/eval_unit`；关联 IN `LambdaExpr`；`merge_column_alias_maps` |
| `filter_stmt.h` | 同上 | 声明 eval API |
| `logical_plan_generator.cpp` | expression, complex-sub-query | JOIN 谓词 interleaving；OR 分组；`filter_unit_to_expr` 保留 LambdaExpr |

### 批次 B — UPDATE / NULL

| 改动文件 | 题目 | 说明 |
|----------|------|------|
| `update_executor.cpp` | null, update-select | `FilterStmt::eval` 支持 `IS NULL` WHERE；标量子查询 SET；**table->delete/insert**（与平台 update 满分一致） |

### 批次 C — DDL

| 改动文件 | 题目 | 说明 |
|----------|------|------|
| `yacc_sql.y`, `parse_defs.h` | create-view, CTAS | `CREATE VIEW` / `CREATE TABLE AS SELECT` |
| `create_view_stmt.*`, `create_view_executor.*` | create-view | 物化视图 |
| `create_table_stmt.*`, `create_table_executor.cpp` | CTAS | 转调 materialize |
| `materialize_select.*` | 两者 | 推断 schema + 执行 SELECT + 插入 |
| `command_executor.cpp`, `stmt.cpp` | 两者 | 路由 |

### 批次 D — 表达式 / 函数

| 改动文件 | 题目 | 说明 |
|----------|------|------|
| `expression.cpp` | expression, alias | 除法 double+`isfinite`→NULL；关联字段解析（071909d 已有基础） |
| `parse.cpp` | function | 无 FROM 标量 SELECT→CALC（071909d 已有） |

### 批次 E — 存储 / 索引

| 改动文件 | 题目 | 说明 |
|----------|------|------|
| `mvcc_trx.cpp` | multi-index | `delete_record` 经 `visit_record` 持久化 end_xid |
| `table.cpp` | unique, text, null | 唯一检查跳过软删索引项；字符串超长返回错误防崩溃 |
| `index.h` | unique | 复合唯一辅助 |

### 批次 F — 其他

| 改动文件 | 说明 |
|----------|------|
| `select_stmt.cpp` | 列别名映射传入 FilterStmt |
| `execute_stage.cpp` | 小调整 |

---

## 4. 建议提交策略（用户 push）

```text
commit 1 (在 071909d 上)
  feat: create-view and create-table-as-select via materialize_select

commit 2
  fix: update null/is-null where and update-select via FilterStmt::eval

commit 3
  fix: mvcc soft-delete persistence and unique index stale entry skip

commit 4
  fix: join predicate interleaving and float div null on overflow

commit 5
  fix: text insert length guard
```

每 commit 后平台提测，**不要**再合并 `4f47e69` 的列序/find_cell 改动。

---

## 5. 本地验证说明

- 命令：`python3 platform-info/_local/sockrun.py <file>`
- **function**：`function.sql` → `36|12` ✓
- **update/null/CTAS**：本地 MVCC 与平台行为不一致（纯 071909d 本地 `UPDATE SET num=99` 也不生效），**以平台为准**
- 应用 `_local/*.sql` 前建议清空数据目录或换库名

---

## 6. 预期得分（保守）

| 阶段 | 新增满分题 | 预估总分 |
|------|-----------|----------|
| 071909d 基线 | — | 260 |
| +null, update-select, create-view, CTAS | +4 | 320 |
| +function, expression, unique, multi-index | +3~4 | 380~400 |
| +alias, complex-sub-query, text | +1~3 | 400~460 |

实际以平台 SQL 轮换后的 detail 为准。

---

## 7. 参考文件

- `platform-info/071909d/summary.md` — 071909d 平台快照
- `platform-info/9d6888d/summary.md` — 9d6888d 回归说明
- `platform-info/fixes-log.md` — commit 映射日志
- `platform-info/FIX-PLAN.md` — 历史修复计划（基线曾写 768c08c，现以本文为准）

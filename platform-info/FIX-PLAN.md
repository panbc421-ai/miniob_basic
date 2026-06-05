# 修复计划 — 基线 `768c08c`（240/500）

**依据**：最新平台 detail（与 `768c08c/summary.md` 同结构，部分 SQL/期望值轮换）  
**原则**：按 [`ANALYSIS-REVIEW.md`](ANALYSIS-REVIEW.md) 检查清单 — 只信所得分、用当前 detail SQL、每批 commit 可定位  
**目标**：12 题 260 分 → 尽量满分，且不回退已满分 13 题

---

## 1. 当前状态

| 指标 | 值 |
|------|-----|
| 基线 commit | `768c08c` |
| 总分 | 240 / 500 |
| 满分题 | 13 |
| 待修复 | 12 题，260 分 |

---

## 2. 失败题分析（最新 detail）

| # | 题目 | 分 | 关键 SQL / 现象 | 根因假设 | 修复方向 | 风险 |
|---|------|-----|----------------|----------|----------|------|
| 1 | **null** | 20 | `SELECT min(num) FROM null_table3` 期望 NULL，得 `-2147483648` | 无有效值时 MIN 输出哨兵转 int；未跳过 NULL 行 | 聚合算子 `counts==0` 返回 NULL；跳过 `val.is_null()` | 低 |
| 2 | **function** | 20 | `length('...')` 期望 `46\|62`，无输出 | CALC/FunctionExpr 路径或别名未输出 | 查 parse→CALC→CalcPhysicalOperator；补 `_local/function2.sql` | 低 |
| 3 | **group-by** | 20 | `having sum(score)>3.861` 多出 id=2,7,8 | **HAVING 未实现**（无 token/字段） | lex+yacc HAVING；GroupBy 后过滤 | 中 |
| 4 | **alias** | 20 | `count(*) as num, avg(t1.col1) score` 期望 `47\|43.66`，得 `0` | ① `agg_expr` 无 `alias_opt` ② `select_stmt` 忽略用户别名 ③ 可能 JOIN 聚合计数错 | yacc `agg_expr alias_opt`；`agg.alias=sel_expr.alias` | 中 |
| 5 | **update-select** | 20 | `SET t_name=(select...) WHERE col1=5 and col2=9` FAILURE | 子查询 SET + 复合 WHERE 解析/求值/类型 | 查 yacc、eval_scalar_subquery、char 赋值 | 中 |
| 6 | **expression** | 20 | 算术表达式 `86.63` 写成 `86.62`（差 0.01） | float 中间精度不足 | 算术用 double；输出 `%.2f` 四舍五入 | 低 |
| 7 | **create-view** | 20 | `create view ...` FAILURE | 未实现 | SCF_CREATE_VIEW + 元数据 + 查询改写 | 中高 |
| 8 | **create-table-select** | 20 | CTAS FAILURE | 未实现 | `CREATE TABLE t AS SELECT` parser+executor | 中高 |
| 9 | **unique** | 20 | 合法 INSERT 判 FAILURE | 唯一索引重复检测误判（复合键？） | 查 `build_record_key`、insert 前扫描 | 中 |
| 10 | **multi-index** | 20 | DELETE id=5,9 后仍出现 | 复合索引 delete_entry 未清干净 | 查 `delete_entry_of_indexes`、多列 key | 中高 |
| 11 | **complex-sub-query** | 30 | OR+标量子查询缺 `33\|27\|99.16` | 标量 OR 条件过滤不全 | 查 Predicate/子查询求值 | 高 |
| 12 | **text** | 20 | 超长 TEXT insert 崩溃 | 越界/页溢出 | TEXT 存储与长度检查 | 高 |

**说明**：`expression` 本轮 detail 已从「聚合算术」换成「纯算术+WHERE」，仍 0 分但修复点不同。

---

## 3. 执行批次（每批 1 commit，便于平台定位）

### 批次 A — 快速得分（+60 潜在）

| 顺序 | commit 目标 | 题目 | 改动要点 |
|------|-------------|------|----------|
| A1 | `fix-null-agg` | null | MIN/MAX/AVG/SUM 无有效值→NULL；跳过 NULL 输入 |
| A2 | `fix-group-by-having` | group-by | HAVING 词法/语法 + GroupBy 后 PostAggTuple 过滤 |
| A3 | `fix-alias-agg` | alias | `agg_expr alias_opt` + 聚合别名输出 |
| A4 | `fix-function-calc` | function | 确保无 FROM length→CALC 全链路 |
| A5 | `fix-expression-precision` | expression | double 算术精度 |

### 批次 B — 中等功能（+60 潜在）

| 顺序 | commit 目标 | 题目 |
|------|-------------|------|
| B1 | `fix-update-select-subquery` | update-select |
| B2 | `feat-create-view` | create-view |
| B3 | `feat-create-table-as-select` | create-table-select |

### 批次 C — 存储/索引（+70 潜在）

| 顺序 | commit 目标 | 题目 |
|------|-------------|------|
| C1 | `fix-unique-index` | unique |
| C2 | `fix-multi-index-delete` | multi-index |
| C3 | `fix-text-overflow` | text |

### 批次 D — 高难度（+30）

| 顺序 | commit 目标 | 题目 |
|------|-------------|------|
| D1 | `fix-complex-subquery-or` | complex-sub-query |

---

## 4. 本地用例对齐（每批必做）

从最新 detail 提取 SQL，写入 `platform-info/_local/`：

| 文件 | 对题 |
|------|------|
| `null_min.sql` | null — `min(num)` 全 NULL |
| `groupby_having.sql` | group-by — having sum>阈值 |
| `alias_agg.sql` | alias — count+avg join |
| `function2.sql` | function — 最新 length 字符串 |
| `expression_arith.sql` | expression — 算术列 |
| `updateselect_subq.sql` | update-select — SET 子查询 |

验证：`python3 platform-info/_local/sockrun.py <file>`

---

## 5. 预期得分路线图

| 阶段 | 累计满分题 | 预估总分 |
|------|-----------|----------|
| 基线 768c08c | 13 | 240 |
| +批次 A | +4~5 | 300~320 |
| +批次 B | +2~3 | 340~380 |
| +批次 C | +2~3 | 380~440 |
| +批次 D | +0~1 | 400~470 |

（实际以平台 detail 为准，SQL 轮换时不得分 ≠ 修复无效。）

---

## 6. 执行检查清单（每批提交前）

- [ ] 改动 SQL 来自**最新 detail**，不是历史 `6ba6d09` detail
- [ ] `_local/*.sql` 已更新并 socket 通过
- [ ] `bash build.sh debug --make -j4` 无 error
- [ ] 语法改动已 `gen_parser.sh`
- [ ] `fixes-log.md` 追加一行 commit 映射
- [ ] 用户 push + 平台提测（agent 不 push）

---

## 7. 当前进度

| 批次 | 状态 |
|------|------|
| A1 null | ✅ 本地 `null_min.sql` → NULL |
| A2 group-by | ✅ 本地 `groupby_having.sql` HAVING 过滤生效 |
| A3 alias | 🔄 yacc `agg_expr alias_opt` 已改，待平台验证 |
| A4 function | 待开始 |
| A5 expression | 待开始 |
| B~D | 待开始 |

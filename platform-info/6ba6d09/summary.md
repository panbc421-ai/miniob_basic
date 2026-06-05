# 平台测试结果 — commit `6ba6d09`

**Commit**: `6ba6d09841d8fbb92f56dd1fdc4da7ff620910fa`
**Message**: fix: 复合索引仅前缀列等值才走IndexScan，避免非前缀扫描崩溃
**总分**: 250 / 500

**图例**（来自题面）：
- `-` 开头 = **应该输出但我们的代码没有输出**（漏 / missing）
- `+` 开头 = **不应该输出但我们的代码多输出了**（多 / extra）

---

## 已通过 (10/25)

| 题目 | 分数 |
|------|------|
| aggregation-func | 10 |
| basic | 10 |
| big-order-by | 30 |
| date | 10 |
| drop-table | 10 |
| join-tables | 20 |
| like | 10 |
| simple-sub-query | 20 |
| update | 10 |
| update-mvcc | 30 |

## 未通过 (15/25, 缺 250 分)

---

### 1. alias (0/20)
```sql
select t1.id as num, t1.col1 as age, t1.feat1, t2.*
from table_alias_1 t1, table_alias_2 t2 where t1.id < t2.id;
```
- 期望：6 行结果（笛卡尔积满足 t1.id<t2.id）
- 实际：`+ FAILURE`（整句失败）
- **根因假设**：`t2.*`（带表前缀的星号展开）未实现 / 别名 `t2` 无法解析到 `table_alias_2`。

---

### 2. big-query (0/30)
```sql
select * from big_query_0;
```
- 期望：全部记录（约 2029 行）
- 实际：`-` 大量行缺失（只返回了部分）
- **根因假设**：大结果集被截断——网络发送缓冲 / 单次响应行数上限 / 翻页未实现。

---

### 3. big-write (0/30)
```sql
select * from big_write_0;
```
- 期望：包含合法重复行（如 id=1031 出现 4 次，是数据本身重复）
- 实际：`-` 这些行缺失
- **根因假设**：同 big-query，大量写入后查询结果不完整（截断或缓冲问题）。

---

### 4. complex-sub-query (0/30)
```sql
select * from csq_1 where feat1 > (select min(csq_2.feat2) from csq_2
  where csq_2.id in (select csq_3.id from csq_3
    where csq_3.col3 <> (select csq_4.col4 from csq_4 where csq_4.id <> csq_1.id)));
```
- 期望：`- FAILURE`（标量子查询返回多行 + `<>`，语义错误，应失败）
- 实际：未输出 FAILURE（可能崩溃无响应 / 返回了错误结果）
- **根因假设**：多层关联子查询 + 标量子查询多行未做校验；需检测标量子查询返回 >1 行时报错。

---

### 5. create-table-select (0/20)
```sql
create table create_table_select_t2 as select * from create_table_select_t1;
```
- 期望：`- SUCCESS`
- 实际：`+ FAILURE`
- **根因假设**：`CREATE TABLE ... AS SELECT` 未实现（parser + executor）。

---

### 6. create-view (0/20)
```sql
create view create_view_v1 as select * from create_view_t1;
```
- 期望：`- SUCCESS`
- 实际：`+ FAILURE`
- **根因假设**：`CREATE VIEW` 未实现。

---

### 7. expression (0/20)
```sql
select count(id) from exp_table where 1/7*3 < 9+col3*col3/9;
select min(col1)+avg(col2)*max(col3)/(max(col4)-4) from exp_table where id<>7/7;
```
- 期望：SQL2 输出 `- 9.61`
- 实际：`+ FAILURE`
- **根因假设**：聚合函数参与算术表达式（`min()+avg()*max()/(max()-4)`）未支持；select 列表里的聚合表达式求值缺失。

---

### 8. function (0/20)
```sql
select length('E2NO0VF4EHPV72MC73YT54585CDTYF22ZWVP') len1, length('W8AOL9JDY025') len2;
```
- 期望：`- 36 | 12`
- 实际：未输出（之前已多次提交修，仍 0 分）
- **根因假设**：无 FROM 的多列标量函数 + 别名输出仍有 bug，需复查 `eb5a518` 后的剩余问题。

---

### 9. group-by (0/20)
```sql
select id, sum(score) from t_group_by group by id having sum(score)>3.2809999999999997;
```
- 期望：仅 sum>3.281 的组 (2,4,5,6,7,8)
- 实际：多输出 `+ 1 | 1.43`、`+ 9 | 3.16`（均 <3.281）
- **根因假设**：`HAVING` 子句未解析/未应用（`SelectSqlNode` 无 having 字段）。

---

### 10. multi-index (0/20)
```sql
SELECT * FROM multi_index3;   -- 在 DELETE id=2、id=6 之后
```
- 实际：多输出 `+ 2 | 2 | 33.35 | Z5Y | 2007-03-20 | 5 | 82`（id=2 已被删除却仍返回）
- **根因假设**：删除后复合索引项未正确清理，IndexScan 仍命中旧项；或 delete 走了某条复合索引未删干净。

---

### 11. null (0/20)
```sql
SELECT * FROM null_table where 9 is null;
```
- 期望：空结果集（常量 9 IS NULL = false，返回 0 行，**不是** 失败）
- 实际：`+ FAILURE`
- **根因假设**：常量 `IS NULL` / `IS NOT NULL` 被错误判定为类型错误而失败；应正常求值为布尔。

---

### 12. order-by (0/20)
```sql
select id from t_order_by_3 order by id;   -- t_order_by_3(id int null, score float)，含 NULL
```
- 期望首部：`- NULL`（NULL 排最前），且值序列与我们不同
- 实际：`+ 2` 排到前面，且多出 `+ 11 + 14 + 29`，缺 `- 2 - 5`
- **根因假设**：ORDER BY 对 NULL 的排序位置错误（NULL 应排最前/最小）；可能 NULL 比较逻辑导致整体排序错乱。

---

### 13. text (0/20)
```sql
insert into text_table values (4,'<超长字符串>');
```
- 实际：`+ ... POLLHUP/POLLERR`（observer 进程崩溃，连接断开）
- **根因假设**：TEXT 类型超长字符串写入越界 / 单页存储溢出导致段错误。

---

### 14. unique (0/20)
```sql
INSERT INTO unique_table2 VALUES (2,3,1,1);
```
- 期望：`- SUCCESS`
- 实际：`+ FAILURE`
- **根因假设**：unique 约束**误判冲突**（对不冲突的插入也拒绝）；可能复合唯一索引 key 比较错误，或把不同 key 当成相同。

---

### 15. update-select (0/20)
```sql
UPDATE Update_select_t1 SET t_name='9UBT4J5', col2=2 WHERE id=5;
SELECT * FROM Update_select_t1;
```
- 实际：所有行 t_name 都变成 '9UBT4J5'（应仅 id=5 的行）
- **根因假设**：`UpdateSqlNode` 仅支持单列单值；多列 SET 解析后丢失 WHERE 过滤 / 错误地对全表生效。需支持多列 UPDATE + 正确的 WHERE。

---

## 修复批次规划（每批一个 commit，便于平台定位）

| 批次 | 题目 | 风险 | 备注 |
|------|------|------|------|
| A | null(常量IS NULL)、group-by(HAVING)、function(复查) | 低-中 | 语义/解析清晰 |
| B | update-select(多列UPDATE)、expression(聚合算术) | 中 | stmt+executor |
| C | alias(t2.*)、order-by(NULL排序) | 中 | project/sort |
| D | create-view、create-table-select | 中 | DDL |
| E | unique(误判)、multi-index(删除残留)、text(崩溃) | 中-高 | 存储/索引 |
| F | complex-sub-query、big-query、big-write | 高 | 子查询/大结果 |

> 注：本地 WSL 无 cmake 且 sudo 需密码，无法本地编译；代码改动靠人工核对 `.test`/平台 detail，由用户 push + 平台提测验证。

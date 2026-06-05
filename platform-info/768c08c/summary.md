# 平台测试结果 — commit `768c08c`（当前基线）

**Commit**: `768c08c`  
**Message**: function: commit no-FROM scalar SELECT -> CALC rewrite (parse stage)  
**总分**: **240 / 500**（detail 所得分逐项相加）  
**满分题数**: 13 / 25  
**更新日期**: 2026-06-05

**图例**（来自题面）：
- `-` 开头 = **应该输出但我们的代码没有输出**（漏 / missing）
- `+` 开头 = **不应该输出但我们的代码多输出了**（多 / extra）

**相对历史基线 `6ba6d09`**：160 → 240（**+80**），新增满分题 `order-by`、`big-query`、`big-write`。

---

## 得分总表

| 题目 | 分数 | 所得分 | 状态 |
|------|------|--------|------|
| aggregation-func | 10 | 10 | ✅ |
| basic | 10 | 10 | ✅ |
| big-order-by | 30 | 30 | ✅ |
| big-query | 30 | 30 | ✅ |
| big-write | 30 | 30 | ✅ |
| date | 10 | 10 | ✅ |
| drop-table | 10 | 10 | ✅ |
| join-tables | 20 | 20 | ✅ |
| like | 10 | 10 | ✅ |
| order-by | 20 | 20 | ✅ |
| simple-sub-query | 20 | 20 | ✅ |
| update | 10 | 10 | ✅ |
| update-mvcc | 30 | 30 | ✅ |
| alias | 20 | 0 | ❌ |
| complex-sub-query | 30 | 0 | ❌ |
| create-table-select | 20 | 0 | ❌ |
| create-view | 20 | 0 | ❌ |
| expression | 20 | 0 | ❌ |
| function | 20 | 0 | ❌ |
| group-by | 20 | 0 | ❌ |
| multi-index | 20 | 0 | ❌ |
| null | 20 | 0 | ❌ |
| text | 20 | 0 | ❌ |
| unique | 20 | 0 | ❌ |
| update-select | 20 | 0 | ❌ |
| **合计** | **500** | **240** | |

---

## 已通过 (13/25)

| 题目 | 分数 | 所得分 |
|------|------|--------|
| aggregation-func | 10 | 10 |
| basic | 10 | 10 |
| big-order-by | 30 | 30 |
| big-query | 30 | 30 |
| big-write | 30 | 30 |
| date | 10 | 10 |
| drop-table | 10 | 10 |
| join-tables | 20 | 20 |
| like | 10 | 10 |
| order-by | 20 | 20 |
| simple-sub-query | 20 | 20 |
| update | 10 | 10 |
| update-mvcc | 30 | 30 |

---

## 未通过 (12/25, 缺 260 分)

---

### 1. alias (0/20)

```sql
select count(*) as num, avg(t1.col1) score
from table_alias_1 t1, table_alias_2 t2 where t1.id < t2.id;
```

- 期望：`- 11 | 37.82`
- 实际：`+ 0`
- **根因假设**：JOIN 上聚合 + 列别名输出异常（与历史基线 `t2.*` 展开是不同测法）。

---

### 2. complex-sub-query (0/30)

```sql
select * from csq_1
where (select max(csq_2.feat2) from csq_2) > feat1
   or (select min(csq_3.col3) from csq_3) < col1;
```

- 期望：含 `- 78 | 96 | 96.05` 等 3 行
- 实际：缺 `78 | 96 | 96.05`
- **根因假设**：标量子查询与外层 OR 条件组合过滤不完整。

---

### 3. create-table-select (0/20)

```sql
create table create_table_select_t2 as select * from create_table_select_t1;
```

- 期望：`- SUCCESS`
- 实际：`+ FAILURE`
- **根因假设**：`CREATE TABLE ... AS SELECT` 未实现。

---

### 4. create-view (0/20)

```sql
create view create_view_v1 as select * from create_view_t1;
```

- 期望：`- SUCCESS`
- 实际：`+ FAILURE`
- **根因假设**：`CREATE VIEW` 未实现。

---

### 5. expression (0/20)

```sql
select count(id) from exp_table where 7/2*9 < 6+col3*col3/6;
select min(col1)+avg(col2)*max(col3)/(max(col4)-6) from exp_table where id<>2/1;
```

- 期望：第二条 `- 7.87`
- 实际：`+ FAILURE`
- **根因假设**：聚合函数参与算术表达式未支持。

---

### 6. function (0/20)

```sql
select length('DFD0WKT9811GTQZD6IKWW') len1, length('65OBQVMCUM1JIU') len2;
```

- 期望：`- 21 | 14`
- 实际：未输出
- **根因假设**：`length()` 或 CALC 执行链在平台仍失败（本地 socket 可通过）。

---

### 7. group-by (0/20)

```sql
select id, sum(score) from t_group_by group by id having sum(score)>4.681;
```

- 期望：不含 id=4、id=7 的组
- 实际：多输出 `+ 4 | 3.16`、`+ 7 | 2.12`
- **根因假设**：`HAVING` 未解析或未正确过滤。

---

### 8. multi-index (0/20)

```sql
SELECT * FROM multi_index3;   -- DELETE id=4、id=7 之后
```

- 实际：多输出已删除行 `+ 4 | 1 | 16.53 | ...`、`+ 7 | 2 | ...`（3 行）
- **根因假设**：DELETE 后复合索引项未清理，IndexScan 仍命中。

---

### 9. null (0/20)

```sql
SELECT min(num) FROM null_table3;
```

- 期望：`- NULL`
- 实际：`+ -2147483648`
- **根因假设**：全 NULL 列上 `MIN()` 未返回 NULL，而是输出哨兵值转 int（`INT_MIN`）。

---

### 10. text (0/20)

```sql
insert into text_table values (4,'<超长字符串>');
```

- 实际：`+ POLLHUP/POLLERR`（observer 崩溃）
- **根因假设**：TEXT 超长写入越界导致段错误。

---

### 11. unique (0/20)

```sql
INSERT INTO unique_table2 VALUES (2,3,1,1);
```

- 期望：`- SUCCESS`
- 实际：`+ FAILURE`
- **根因假设**：唯一约束误判冲突。

---

### 12. update-select (0/20)

```sql
UPDATE Update_select_t2
SET t_name=(select Update_select_t1.t_name from Update_select_t1 where Update_select_t1.id=8)
WHERE col1=8 and col2=9;
```

- 期望：`- SUCCESS`
- 实际：`+ FAILURE`
- **根因假设**：`SET col=(标量子查询)` + 复合 WHERE 组合仍失败。

---

## 后续修复优先级（建议）

| 优先级 | 题目 | 分值 | 备注 |
|--------|------|------|------|
| P0 | null | 20 | `MIN/MAX` 全 NULL → NULL，改动小 |
| P0 | function | 20 | 平台仍 0，需查 length + CALC |
| P1 | group-by | 20 | 补 HAVING |
| P1 | alias | 20 | JOIN 聚合 + 别名 |
| P1 | update-select | 20 | 子查询 SET + WHERE |
| P2 | create-view、create-table-select | 40 | DDL 未实现 |
| P2 | unique、multi-index | 40 | 索引/约束 |
| P3 | expression | 20 | 语法 + 计划重构 |
| P3 | complex-sub-query | 30 | 子查询组合 |
| P3 | text | 20 | 崩溃，风险高 |

# 题目 07 — join-tables（多表连接）

## 架构设计

- 使用 Expression 树统一处理连接条件
- 实现嵌套循环连接（Nested Loop Join）

## 需要修改的文件

| 文件 | 改动内容 |
|------|----------|
| `src/observer/sql/parser/yacc_sql.y` | JOIN 语法解析 |
| `src/observer/sql/stmt/select_stmt.h` | 多表关系存储 |
| `src/observer/sql/operator/join_operator.h/cpp` | Join 物理算子 |
| `src/observer/sql/expr/expression.h` | ON 条件 Expression |

## 实现要点

- 支持 `INNER JOIN`
- 多表 JOIN 按顺序构建算子树
- ON 条件支持多个比较表达式（AND 连接）
- 大数据量时考虑外层缓存优化

## 测试用例

```sql
create table t1(id int, name char(10));
create table t2(id int, score float);
insert into t1 values(1, 'Alice');
insert into t2 values(1, 95.5);
select t1.name, t2.score from t1 inner join t2 on t1.id = t2.id;
-- 期望：Alice | 95.5
```

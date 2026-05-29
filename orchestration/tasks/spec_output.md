# 当前题目: join-tables
# 日期: 2026-05-29

## 平台反馈
- 平台0分，需按指导书完善
- 本地测试 `primary-join-tables` 通过
- 说明平台测试了本地未覆盖的 SQL 语法或边界场景

## 已知缺失功能（基于 MiniOB 标准 join-tables 课程要求）

### 1. 缺少表别名支持
当前语法仅支持 `INNER JOIN table_name ON ...`，不支持别名：
```sql
-- 当前不支持：
SELECT * FROM t1 a JOIN t2 b ON a.id = b.id
SELECT * FROM t1 AS a JOIN t2 AS b ON a.id = b.id
```
这导致无法实现**自连接**（self-join），且平台必然测试别名场景。

### 2. 缺少 LEFT/RIGHT JOIN
当前仅支持 INNER JOIN，指导书一般会要求同时支持 LEFT JOIN（左外连接）。

### 3. 缺少多表逗号连接与 JOIN 混用
`SELECT * FROM t1, t2 INNER JOIN t3 ON t1.id = t3.id` 当前无法解析。

## 指导书输出格式（无指导书 PDF，基于 MiniOB 惯例）
- 字段分隔符: ` | `（空格+竖线+空格）
- 每行一条记录，值以大写下划线形式输出
- 结果末尾附带表头行（MiniOB 框架自动添加）
- NULL 显示: 无相关测试用例

## SQL 需求（需要支持的语法）
```sql
-- 表别名（AS 可选）
SELECT * FROM t1 a JOIN t2 b ON a.id = b.id;
SELECT * FROM t1 AS a JOIN t2 AS b ON a.id = b.id;

-- 左外连接  
SELECT * FROM t1 LEFT JOIN t2 ON t1.id = t2.id;

-- 自连接（依赖别名）
SELECT a.id, b.id FROM t1 a JOIN t1 b ON a.id = b.id WHERE a.id = 1;
```

## 受影响文件
- Parser: yacc_sql.y, yacc_sql.hpp, lex_sql.l, lex_sql.h, lex_sql.cpp
- Parse defs: parse_defs.h (JoinClauseNode 增加表别名字段)  
- Select stmt: select_stmt.h, select_stmt.cpp (表名→别名映射)
- Optimizer: logical_plan_generator.cpp (使用别名解析字段引用)
- Physical plan: physical_plan_generator.cpp, physical_plan_generator.h

## 可能破坏的已有功能
- basic: 基本 SELECT 解析路径共享 relation 列表
- select-tables: 多表查询依赖 relation 字段引用
- expression: 表达式过滤器中的字段引用使用 relation_name

## 防破坏约束
- 不修改已有字段 Query::relation_name 的内部存储结构
- 保持向后兼容：无别名时行为不变
- 别名解析仅在 JOIN 上下文中生效
- 不修改 FilterStmt 的条件编译逻辑
- 所有回归测试通过后再提交

## 建议实现策略
1. 在 JoinClauseNode 中增加 `alias` 字段
2. 在 parser 中支持 `INNER/LEFT JOIN ID opt_alias ON ...`
3. 在 SelectStmt 中建立 alias→table 映射
4. 在 logical_plan_generator 中使用别名映射解析字段引用

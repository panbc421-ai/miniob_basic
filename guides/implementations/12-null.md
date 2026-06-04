# 题目 12 — null（NULL 值支持）

## 核心修改

- Record 存储结构增加 **Bitmap 位图**标记 NULL 字段
- `FieldMeta` 增加 `nullable` 属性

## 需要修改的文件

| 文件 | 改动内容 |
|------|----------|
| `src/observer/storage/field/field_meta.h` | 添加 `nullable` 标志 |
| `src/observer/storage/record/record_manager.cpp` | 添加位图存储 |
| `src/observer/sql/expr/value.h` | NULL 值处理 |
| `src/observer/sql/operator/predict_operator.cpp` | NULL 比较逻辑 |

## NULL 语义规则

| 场景 | 行为 |
|------|------|
| `NULL = NULL` | 返回 FALSE（不是 TRUE） |
| `NULL <> 1` | 返回 NULL/FALSE |
| `IS NULL` | 唯一能正确检测 NULL 的谓词 |
| `COUNT(字段)` | 忽略 NULL |
| `COUNT(*)` | 包含 NULL 行 |
| `AVG / SUM` | 忽略 NULL |

## 测试用例

```sql
create table t(id int, val int nullable);
insert into t values(1, null);
insert into t values(2, 10);
select * from t where val is null;
-- 期望：1 | NULL
select count(val), count(*) from t;
-- 期望：1 | 2
```

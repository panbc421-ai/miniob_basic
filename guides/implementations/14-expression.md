# 题目 14 — expression（表达式）

> 建议优先完成本题，可显著简化后续聚合、子查询等功能。

## 需要修改的文件

| 文件 | 改动内容 |
|------|----------|
| `src/observer/sql/expr/expression.h/cpp` | Expression 基类扩展 |
| `src/observer/sql/expr/arithmetic_expression.h/cpp` | 算术表达式实现 |
| `src/observer/sql/operator/calc_operator.cpp` | 表达式计算算子 |

## 支持的运算

- 四则运算：`+ - * /`
- 类型自动转换：`int` ↔ `float`
- 除零处理：返回 `NULL` 类型（不崩溃）
- 括号优先级

## 测试用例

```sql
select id + 1, id * 2.0 from t;
select (a + b) / c from t where c <> 0;
```

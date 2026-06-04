# 题目 02 — date（日期类型支持）

## 需要修改的文件

| 文件 | 改动内容 |
|------|----------|
| `src/observer/sql/parser/lex_sql.l` | 添加 `DATE` 关键字 |
| `src/observer/sql/parser/yacc_sql.y` | 添加 DATE 类型解析规则 |
| `src/observer/sql/parser/parse_defs.h` | 添加 `DateType` 枚举 |
| `src/observer/sql/expr/value.h/cpp` | 实现 Date 类型 Value |
| `src/observer/storage/field/field_meta.h` | 添加 DATE 类型存储 |
| `src/observer/storage/record/record_manager.cpp` | 序列化/反序列化 Date |
| `src/observer/storage/index/bplus_tree.cpp` | Date 类型的索引比较 |

## 关键实现要点

- **存储格式**：`int32_t`（以整数编码 `YYYYMMDD`，便于比较）
- **闰年判断**：正确处理 2 月 29 日
- **非法日期**：输入非法日期时返回 `FAILURE`
- **日期范围**：注意 int32 溢出边界
- **索引比较**：使用整数直接比较，无需额外处理

## 测试用例

```sql
create table t(id int, birth date);
insert into t values(1, '2000-01-01');
insert into t values(2, '1990-06-15');
select * from t where birth > '1995-01-01';
-- 期望输出：1 | 2000-01-01
```

## 常见坑

- 输出格式必须为 `YYYY-MM-DD`，不能是时间戳整数
- `'2000-02-29'` 合法（2000 年是闰年），`'1900-02-29'` 非法

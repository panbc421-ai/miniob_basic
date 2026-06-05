# Platform result: `4f47e69`

**Commit**: `fix: group-by SUM regression and SELECT alias column order`  
**总分**: **~230 / 500**（较 071909d **−30**）  
**结论**: **勿作基线**；仅作回归复盘参考。

## 相对 071909d 的回归

| 题目 | 071909d | 4f47e69 | 根因 |
|------|---------|---------|------|
| aggregation-func | 10 | 0 | AVG `50.63`→`50.62`（`find_cell` 回退） |
| group-by | 20 | 0 | `having count(*)>1` 无输出（schema/列序） |
| expression | 0 | 0 | 列顺序对调 |
| create-view / CTAS | 0 | 0 | 本地 SUCCESS→平台 FAILURE |
| update-select | 0 | 0 | SUCCESS→FAILURE |

## 相对 9d6888d

- 继承 group-by / alias 问题
- 额外丢失 aggregation-func
- expression 从「多行」变为「列错位」

## Detail 摘要（用户反馈）

- **aggregation-func**: `50.63\|1\|39ZS\|16` → `50.62\|...`
- **alias**: `t1.id as num ... id in (...)` FAILURE
- **group-by**: 期望 7 行 max(score)，无输出
- **expression**: 四行结果列顺序整体错位
- **null**: `8\|NULL\|NULL` → `8\|NULL\|N8`
- **text**: observer 崩溃 POLLHUP

## 教训

1. 不要全局对调 `query_fields` / `select_exprs` 投影顺序  
2. 不要无差别 `find_cell` 回退  
3. `filter_stmt` 错误路径改 `RC::SUCCESS` 会伤及 update-select  
4. GROUP BY schema 改动需 isolated 测试

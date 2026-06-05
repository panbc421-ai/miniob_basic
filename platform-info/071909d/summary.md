# Platform result: `071909d`

**Commit**: `fix: e43d2d6 regressions for CHAR min, agg arithmetic, and alias IN`  
**Total score**: **260 / 500** (14 full passes + aggregation-func)  
**Delta vs `e43d2d6` (250)**: +aggregation-func +10; alias no longer crashes but still 0

## Passed (所得分 = 260)

| Test | Score |
|------|-------|
| aggregation-func | 10 |
| basic | 10 |
| big-order-by | 30 |
| big-query | 30 |
| big-write | 30 |
| date | 10 |
| drop-table | 10 |
| group-by | 20 |
| join-tables | 20 |
| like | 10 |
| order-by | 20 |
| simple-sub-query | 20 |
| update | 10 |
| update-mvcc | 30 |

## Failed (11)

| Test | Score | Symptom |
|------|-------|---------|
| alias | 0/20 | `t1.id as num ... where id in (select ... where num > t2.id)` → FAILURE (no crash) |
| expression | 0/20 | Join expression SELECT extra rows (`6\|0.29`, `9\|0.29`) |
| function | 0/20 | `length(...)` expects `32 \| 5` |
| null | 0/20 | After `UPDATE SET name=null WHERE num is null`, row 8: `NULL \| N8` |
| update-select | 0/20 | `SET col2=(select ...) WHERE id=5` → FAILURE |
| complex-sub-query | 0/30 | Missing row `54 \| 31 \| 90.79` |
| create-table-select | 0/20 | CTAS FAILURE |
| create-view | 0/20 | CREATE VIEW FAILURE |
| multi-index | 0/20 | Rows remain after composite-key DELETE |
| text | 0/20 | Long TEXT insert crash |
| unique | 0/20 | Duplicate unique insert should FAIL |

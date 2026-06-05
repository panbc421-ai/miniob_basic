# Platform result: `e43d2d6`

**Commit**: `fix: null agg NULL semantics, HAVING clause, and agg column aliases`  
**Total score**: **250 / 500** (14 full passes)  
**Delta vs `768c08c` (240)**: +group-by +20, −aggregation-func −10

## Passed (14 × 所得分)

| Test | Score |
|------|-------|
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
| aggregation-func | 0/10 | `SELECT min(addr)` → NULL (expected `216C`) — CHAR MIN used `counts==0` instead of `has_str_min` |
| alias | 0/20 | SIGSEGV in `FilterStmt::create_filter_unit` — IN subquery + outer column alias `num`; nullptr `inner_stmt` fall-through |
| expression | 0/20 | SIGSEGV in `resolve_expression` — agg arithmetic UAF in `collect_aggregations` |
| function | 0/20 | No output (`length` scalar) |
| null | 0/20 | After `UPDATE SET name=null WHERE num is null`, row 8: `NULL \| N8` not `NULL \| NULL` |
| update-select | 0/20 | `SET t_name=(select...) WHERE col1=6 and col2=5` → FAILURE |
| complex-sub-query | 0/30 | Missing row `65 \| 48 \| 98.27` |
| create-table-select | 0/20 | SUCCESS expected, got FAILURE |
| create-view | 0/20 | SUCCESS expected, got FAILURE |
| multi-index | 0/20 | Extra rows after DELETE (index residue) |
| text | 0/20 | Crash on very long TEXT insert |
| unique | 0/20 | Duplicate unique index insert should FAIL |

## Post-`e43d2d6` local fixes (uncommitted)

1. **aggregation-func regression**: CHAR/DATE MIN/MAX NULL via `has_str_min`/`has_str_max`; increment counts for char min/max in agg + group-by.
2. **expression crash**: Fix `collect_aggregations` unique_ptr UAF; place Agg under Project for `select_exprs` agg arithmetic; `AggregationTuple::find_cell` field-name match; schema from `select_exprs`.
3. **alias crash**: Column alias map for correlated subqueries; detect correlation via outer select aliases; null `inner_stmt` guard; fallback to correlated path on failed inner `SelectStmt::create`.
4. **Parser**: `agg_expr` in `expression` for SELECT arithmetic; `agg_expr comp_op expression` in `condition` for HAVING only.

## Local verification (WSL)

- `agg_min_char.sql`: `min(addr)` → `7` (not NULL) ✅
- `expression.sql`: agg arithmetic → `5.77`, no crash ✅
- `alias_in.sql`: IN + outer alias `num`, no crash ✅
- `groupby_having.sql`: HAVING still OK ✅

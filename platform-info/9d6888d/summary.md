# Platform result: `9d6888d`

**Total score**: **260 / 500** (same as `071909d`; group-by regressed to 0)

## Regressions vs `071909d`

| Test | Was | Now |
|------|-----|-----|
| group-by | 20 | 0 — `sum(score)` all NULL; schema `id \| id` |

## Still failing

alias (wrong column order / missing rows for `t2.*` join), expression, function, null, update-select, complex-sub-query, create-view, ctas, multi-index, text, unique

## Post-`9d6888d` local fixes (uncommitted)

1. Implicit vs explicit SELECT alias (`alias == field_name` only, before `set_name`)
2. GROUP BY schema uses `agg_fields` not duplicate `select_exprs`
3. Project column order: `select_exprs` before `query_fields`
4. `find_cell` fallback without table name in group-by / agg operators

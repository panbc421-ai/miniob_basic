# MiniOB 实验调度计划

<!-- ORIGIN_COMMIT: f17186e1a72f747fb36e5ee19981caf9a8c4d81f -->
<!-- LAST_SYNC: 2026-05-29 -->

## 回归基线

| 功能 | 测试用例 | 状态 |
|------|----------|------|
| basic | basic | ✅ pass |
| AGGREGATION-FUNC | primary-aggregation-func | ✅ pass |
| DATE | primary-date | ✅ pass |
| DROP TABLE | primary-drop-table | ✅ pass |
| LIKE | primary-like | ✅ pass |
| ORDER-BY (big-order-by) | primary-order-by | ✅ pass |

## 必做题

| 题目 | 分值 | 状态 |
|------|------|------|
| 优化buffer pool | 10 | ⏭️ SKIP (无统一测试) |
| select-meta | 10 | ✅ 已完成 |
| drop-table | 10 | ✅ 已完成 |
| update | 10 | ✅ 已完成 |
| date | 10 | ✅ 已完成 |
| select-tables | 10 | ✅ 已完成 |
| aggregation-func | 10 | ✅ 已完成 |

## 选做题

| 题目 | 分值 | 状态 |
|------|------|------|
| order-by | 10 | ✅ 已完成 |
| like | 10 | ✅ 已完成 |
| join-tables | 20 | ❌ 平台0分，需按指导书修复 |
| expression | 20 | ❌ 平台0分，复杂表达式失败 |
| insert | 10 | ⬜ 未实现 |
| unique | 10 | ⬜ 未实现 |
| null | 10 | ⬜ 未实现 |
| simple-sub-query | 10 | ⬜ 未实现 |
| multi-index | 20 | ⬜ 未实现 |
| text | 20 | ⬜ 未实现 |
| complex-sub-query | 20 | ⬜ 未实现 |
| group-by | 20 | ⬜ 未实现 |

# MiniOB 实验调度计划

<!-- GUIDE: 基于MiniOB的数据库管理系统设计与实现指导书_2026.pdf -->
<!-- LAST_SYNC: 2026-05-29 -->
<!-- ROUND: 3 (platform feedback) -->

## 平台验证结果
- ✅ 已确认: aggregation-func, basic, date, drop-table, join-tables, like, update, update-mvcc, big-order-by
- ❌ 需修复: simple-sub-query, unique, order-by, group-by

## 全部题目（指导书 1.1 ~ 1.25）

| 编号 | 题目 | 测试用例 | 状态 |
|------|------|----------|------|
| 1.1 | basic | basic | ✅ 已完成 |
| 1.2 | date | primary-date | ✅ 已完成 |
| 1.3 | drop-table | primary-drop-table | ✅ 已完成 |
| 1.4 | update | primary-update | ✅ 已完成 |
| 1.5 | aggregation-func | primary-aggregation-func | ✅ 已完成 |
| 1.6 | like | primary-like | ✅ 已完成 |
| 1.7 | join-tables | primary-join-tables | ✅ 已完成 |
| 1.8 | simple-sub-query | primary-simple-sub-query | ❌ 需修复 (platform data mismatch) |
| 1.9 | function | self-test | ✅ 已实现 |
| 1.10 | multi-index | primary-multi-index | ❌ (composite key bug) |
| 1.11 | unique | primary-unique | ❌ 需修复 (multi-col unique) |
| 1.12 | null | primary-null | ❌ (storage crash) |
| 1.13 | update-select | self-test | ⬜ 自测 |
| 1.14 | expression | primary-expression | ✅ 已完成 |
| 1.15 | alias | self-test | ⬜ 自测 |
| 1.16 | text | primary-text | ⬜ |
| 1.17 | order-by | primary-order-by | ✅ 已完成 |
| 1.18 | group-by | primary-group-by | ✅ 已完成 |
| 1.19 | create view | self-test | ⬜ 自测 |
| 1.20 | create-table-select | self-test | ⬜ 自测 |
| 1.21 | complex-sub-query | primary-complex-sub-query | ❌ R3 (correlated crash) |
| 1.22 | update-mvcc | self-test | ⬜ 自测 |
| 1.23 | big-query | self-test | ⬜ 自测 |
| 1.24 | big-write | self-test | ⬜ 自测 |
| 1.25 | big-order-by | primary-order-by (big) | ✅ 已完成 |

## 修复优先级
1. NULL/NOT NULL column modifiers (fixes group-by + order-by platform failures)
2. Multi-column UNIQUE index (fixes unique platform failure)
3. simple-sub-query platform data handling
4. All remaining ⬜ experiments in order

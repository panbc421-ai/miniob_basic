# MiniOB 实验调度计划

<!-- GUIDE: 基于MiniOB的数据库管理系统设计与实现指导书_2026.pdf -->
<!-- LAST_SYNC: 2026-05-29 -->
<!-- ROUND: 2 -->

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
| 1.8 | simple-sub-query | primary-simple-sub-query | ✅ 已完成 |
| 1.9 | function | — 无测试文件 | N/A |
| 1.10 | multi-index | primary-multi-index | ❌ R2 (composite key bug) |
| 1.11 | unique | primary-unique | ✅ 已完成 |
| 1.12 | null | primary-null | ❌ R2 (storage crash) |
| 1.13 | update-select | — 无测试文件 | N/A |
| 1.14 | expression | primary-expression | SKIP |
| 1.15 | alias | — 无测试文件 | N/A |
| 1.16 | text | primary-text | ⬜ |
| 1.17 | order-by | primary-order-by | ✅ 已完成 |
| 1.18 | group-by | primary-group-by | 🔄 实现中 |
| 1.19 | create view | — 无测试文件 | N/A |
| 1.20 | create-table-select | — 无测试文件 | N/A |
| 1.21 | complex-sub-query | primary-complex-sub-query | ⬜ |
| 1.22 | update-mvcc | — 无测试文件 | N/A |
| 1.23 | big-query | — 无测试文件 | N/A |
| 1.24 | big-write | — 无测试文件 | N/A |
| 1.25 | big-order-by | primary-order-by (big) | ✅ 已完成 |

## 补充（非题目编号的测试文件）
| 测试用例 | 说明 | 状态 |
|----------|------|------|
| primary-insert | INSERT 基础功能 | ✅ 通过 |
| primary-select-meta | SELECT 元数据 | ✅ 通过 |
| primary-select-tables | SELECT 多表 | ✅ 通过 |

## 测试文件覆盖情况
- 有 .test 文件: basic, primary-date, primary-drop-table, primary-update, primary-aggregation-func, primary-like, primary-join-tables, primary-simple-sub-query, primary-multi-index, primary-unique, primary-null, primary-expression, primary-group-by, primary-text, primary-order-by, primary-insert, primary-select-meta, primary-select-tables, primary-complex-sub-query
- 无 .test 文件: function, update-select, alias, create-view, create-table-select, update-mvcc, big-query, big-write

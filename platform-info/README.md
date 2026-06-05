# platform-info

平台测试记录与本地复现脚本。

## 当前基线

| 项目 | 值 |
|------|-----|
| **Commit** | `768c08c` |
| **总分** | **240 / 500** |
| **满分题** | 13 / 25 |
| **详情** | [`768c08c/summary.md`](768c08c/summary.md) |

## 历史基线

| Commit | 总分 | 说明 |
|--------|------|------|
| `6ba6d09` | 160 / 500 | [`6ba6d09/summary.md`](6ba6d09/summary.md) |

`6ba6d09` → `768c08c` 得分变化：**+80**（`order-by` +20，`big-query` +30，`big-write` +30）。

## 其他文件

| 文件 | 用途 |
|------|------|
| [`FIX-PLAN.md`](FIX-PLAN.md) | **修复计划**：12 题分析、批次执行顺序、本地用例对齐 |
| [`ANALYSIS-REVIEW.md`](ANALYSIS-REVIEW.md) | **分析复盘**：此前判断、分析错误、25 题对比、同题 SQL 差异 |
| [`fixes-log.md`](fixes-log.md) | commit ↔ 题目改动映射 |
| [`CHANGELOG-6ba6d09-to-768c08c.md`](CHANGELOG-6ba6d09-to-768c08c.md) | 本轮 5 个 commit 详细说明 |
| [`_local/`](_local/) | 本地 SQL 用例 + `sockrun.py`（平台 socket 协议） |

## 计分规则

以平台 detail 中每题 **所得分** 列为准，25 题相加 = 总分。  
「满分题」= 所得分等于该题分数。

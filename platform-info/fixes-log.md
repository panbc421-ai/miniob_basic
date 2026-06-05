# 修复日志（commit ↔ 改动 映射）

> 每条记录一个 commit 及其针对的题目/改动，便于平台提测后定位回归。  
> **当前基线**：[`071909d`](071909d/summary.md)（**260/500**，14 题满分）。  
> **勿用**：`4f47e69`（~230）、`9d6888d`（group-by 回归）。详见 [`COMMIT-TRACKER.md`](COMMIT-TRACKER.md)。

| commit | 题目 | 改动摘要 | 平台验证 |
|--------|------|----------|----------|
| `6ba6d09` | (历史基线) | 160/500，10 题满分 | — |
| `68a7d3e` | null | 新增 `IS NULL` / `IS NOT NULL` 词法+语法+求值 | ❌ 仍 0（平台测 `min(num)`→NULL，非 IS NULL） |
| `d364437` | order-by | sort 比较器 NULL 视为最小 | ✅ 0→20 |
| `f1ebb06` | update-select | 多列 SET + 标量子查询；executor 用 make_record | ❌ 仍 0（平台换了 SQL） |
| `7fd131d` | alias | `rel.*` 星号展开 | ❌ 仍 0（平台换了 SQL，测 JOIN 聚合） |
| `768c08c` | function | 无 FROM 标量 SELECT→CALC（`parse.cpp`） | ❌ 仍 0；**此后作为新基线** |

完整说明见 [`CHANGELOG-6ba6d09-to-768c08c.md`](CHANGELOG-6ba6d09-to-768c08c.md)。  
分析错误与对比复盘见 [`ANALYSIS-REVIEW.md`](ANALYSIS-REVIEW.md)。

## 本批工作区改动（相对 `071909d`，待 commit）

| 范围 | 题目 | 状态 |
|------|------|------|
| filter_stmt eval + 关联 IN | alias, null, update-select, complex-sub-query | 已实现，待平台 |
| create-view + CTAS + materialize | create-view, create-table-select | 已实现，待平台 |
| mvcc delete visit_record | multi-index | 已实现，待平台 |
| unique 软删索引项跳过 | unique | 已实现，待平台 |
| join 谓词 interleaving + div NULL | expression | 已实现，待平台 |
| text 超长拒绝 | text | 已实现，待平台 |
| function CALC | function | 071909d 已有 + 本地通过 |

## 待修复（相对基线 `071909d`，11 题 240 分）

| 题目 | 分值 | 优先级 |
|------|------|--------|
| null | 20 | P0 — `MIN` 全 NULL 返回 NULL |
| function | 20 | P0 — length + CALC 平台路径 |
| group-by | 20 | P1 — HAVING |
| alias | 20 | P1 — JOIN 聚合别名 |
| update-select | 20 | P1 — 子查询 SET |
| create-view | 20 | P2 |
| create-table-select | 20 | P2 |
| unique | 20 | P2 |
| multi-index | 20 | P2 |
| expression | 20 | P3 |
| complex-sub-query | 30 | P3 |
| text | 20 | P3 |

## 环境说明

- 本地：WSL Ubuntu-22.04 + gcc 11.4，`build.sh init` 依赖在 `/usr/local`
- 测试：`platform-info/_local/sockrun.py`（unix-socket + PLAIN 协议）
- `big-query` / `big-write` 在 `6ba6d09`→`768c08c` 间意外满分（+60），非上述 commit 明确修复内容

# 修复日志（commit ↔ 改动 映射）

> 每条记录一个 commit 及其针对的题目/改动，便于平台提测后定位回归。
> 基线平台结果见 `6ba6d09/summary.md`（250/500）。

| commit | 题目 | 改动摘要 | 本地验证 |
|--------|------|----------|----------|
| `6ba6d09` | (基线) | 平台 250/500，15 题未过 | — |
| `68a7d3e` | null | 新增 `IS NULL` / `IS NOT NULL` 词法+语法+求值；常量 `9 is null` 不再 FAILURE 而返回空集 | ✅ 本地 socket 复现全部通过（含 count/avg 跳过 null），回归无影响 |
| (待提交) | order-by | sort 比较器增加 NULL 处理：NULL 视为最小，升序排最前、降序排最后 | ✅ `order by id` 输出 `NULL,NULL,2,2,5,6,6,22` |

## 重要环境说明
- 平台测试很可能曾因 **构建 OOM** 失败而跑的是旧二进制（commit 链中有 `build.sh -j4 避免平台 OOM`、`触发平台重新编译`）。`function` 在本地 CLI 和 socket 两种协议下都输出 `36 | 12` 正确，但平台显示 0 分——疑似平台未成功构建 6ba6d09。
- 本地构建/测试环境：WSL Ubuntu-22.04 + gcc 11.4，依赖已 `build.sh init` 装入 `/usr/local`。
- 测试方式：`platform-info/_local/sockrun.py`（忠实复刻平台 unix-socket + PLAIN 协议）。

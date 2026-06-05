# 修复日志（commit ↔ 改动 映射）

> 每条记录一个 commit 及其针对的题目/改动，便于平台提测后定位回归。
> 基线平台结果见 `6ba6d09/summary.md`（250/500）。

| commit | 题目 | 改动摘要 | 本地验证 |
|--------|------|----------|----------|
| `6ba6d09` | (基线) | 平台 250/500，15 题未过 | — |
| `68a7d3e` | null | 新增 `IS NULL` / `IS NOT NULL` 词法+语法+求值；常量 `9 is null` 不再 FAILURE 而返回空集 | ✅ 本地 socket 复现全部通过（含 count/avg 跳过 null），回归无影响 |
| `d364437` | order-by | sort 比较器增加 NULL 处理：NULL 视为最小，升序排最前、降序排最后 | ✅ `order by id` 输出 `NULL,NULL,2,2,5,6,6,22` |
| `f1ebb06` | update-select | UPDATE 支持多列 SET（`SET a=v, b=v WHERE ...`）及 `col=(标量子查询)`；UpdateSqlNode 改为 assignments 列表，executor 用 make_record 重建行（正确处理 NULL/char 填充/索引） | ✅ 多列只改命中行、单列回归正常、子查询取值正确 |
| (待提交) | alias | 支持 `rel.*` 星号展开（语法 `ID DOT '*'` + select_stmt 展开为该表全部列）。**行值已正确**（6 行匹配） | ⚠️ 表头：显式别名(num/age)与 t2.* 的列名格式待平台结果确认后精确对齐；当前输出 `表名.列名` |

## 重要环境说明
- 平台测试很可能曾因 **构建 OOM** 失败而跑的是旧二进制（commit 链中有 `build.sh -j4 避免平台 OOM`、`触发平台重新编译`）。`function` 在本地 CLI 和 socket 两种协议下都输出 `36 | 12` 正确，但平台显示 0 分——疑似平台未成功构建 6ba6d09。
- 本地构建/测试环境：WSL Ubuntu-22.04 + gcc 11.4，依赖已 `build.sh init` 装入 `/usr/local`。
- 测试方式：`platform-info/_local/sockrun.py`（忠实复刻平台 unix-socket + PLAIN 协议）。

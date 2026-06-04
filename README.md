# MiniOB AutoPilot

自动完成 OceanBase 训练营 25 道题目的闭环系统。

## 核心流程

```
读取题库 → 开发实现 → Git Push → 自动提交 → 读取结果 → 分析失败 → 修复代码 → 重新提交
    ↑                                                                           ↓
    └─────────────────────────── 循环直至通过 ──────────────────────────────────┘
```

## 关键约束

| 约束 | 说明 |
|------|------|
| 严格顺序 | 25 题必须按顺序完成，不可跳题 |
| 10 分钟冷却 | 提交失败后等待 600 秒再次提交 |
| 提交前必须 push | `git push` 成功后再触发提交 |
| 状态持久化 | `.progress.json` 记录每题进度 |

## 文件结构

```
miniob-autopilot/
├── README.md                    # 本文件
├── .progress.json               # 进度状态（运行时生成）
├── .cursor/rules/
│   ├── autopilot.mdc            # 核心 Cursor 规则
│   └── develop-question.mdc     # 题目开发规则
├── automation/
│   ├── main_orchestrator.py     # 主控制循环
│   ├── submitter.py             # OceanBase 自动提交
│   ├── git_manager.py           # Git 操作封装
│   ├── analyzer.py              # 失败结果分析
│   └── logger.py                # 日志工具
├── guides/
│   ├── questions.md             # 25 道题目清单与提示
│   └── implementations/         # 各题实现指南
│       ├── 02-date.md
│       ├── 07-join-tables.md
│       ├── 12-null.md
│       └── 14-expression.md
└── logs/                        # 运行日志（运行时生成）
```

## 快速启动

```bash
# 1. 安装依赖
pip install playwright
playwright install chromium

# 2. 启动 AutoPilot
python automation/main_orchestrator.py
```

## 预期耗时

开发时间 + 25 题 × 10 分钟冷却 ≈ **5–6 小时**

> ⚠️ 训练营可能出现验证码等阻塞，建议保留人工监督选项。

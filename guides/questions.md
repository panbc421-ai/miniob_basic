# 题目清单与进度管理

## `.progress.json` 模板

```json
{
  "project": "MiniOB AutoPilot",
  "version": "1.0.0",
  "start_time": "",
  "current_question": 1,
  "questions": [
    {"id": 1,  "name": "basic",              "status": "pending", "passed_time": null, "retry_count": 0},
    {"id": 2,  "name": "date",               "status": "pending", "passed_time": null, "retry_count": 0},
    {"id": 3,  "name": "drop-table",         "status": "pending", "passed_time": null, "retry_count": 0},
    {"id": 4,  "name": "update",             "status": "pending", "passed_time": null, "retry_count": 0},
    {"id": 5,  "name": "aggregation-func",   "status": "pending", "passed_time": null, "retry_count": 0},
    {"id": 6,  "name": "like",               "status": "pending", "passed_time": null, "retry_count": 0},
    {"id": 7,  "name": "join-tables",        "status": "pending", "passed_time": null, "retry_count": 0},
    {"id": 8,  "name": "simple-sub-query",   "status": "pending", "passed_time": null, "retry_count": 0},
    {"id": 9,  "name": "function",           "status": "pending", "passed_time": null, "retry_count": 0},
    {"id": 10, "name": "multi-index",        "status": "pending", "passed_time": null, "retry_count": 0},
    {"id": 11, "name": "unique",             "status": "pending", "passed_time": null, "retry_count": 0},
    {"id": 12, "name": "null",               "status": "pending", "passed_time": null, "retry_count": 0},
    {"id": 13, "name": "update-select",      "status": "pending", "passed_time": null, "retry_count": 0},
    {"id": 14, "name": "expression",         "status": "pending", "passed_time": null, "retry_count": 0},
    {"id": 15, "name": "alias",              "status": "pending", "passed_time": null, "retry_count": 0},
    {"id": 16, "name": "text",               "status": "pending", "passed_time": null, "retry_count": 0},
    {"id": 17, "name": "order-by",           "status": "pending", "passed_time": null, "retry_count": 0},
    {"id": 18, "name": "group-by",           "status": "pending", "passed_time": null, "retry_count": 0},
    {"id": 19, "name": "create-view",        "status": "pending", "passed_time": null, "retry_count": 0},
    {"id": 20, "name": "create-table-select","status": "pending", "passed_time": null, "retry_count": 0},
    {"id": 21, "name": "complex-sub-query",  "status": "pending", "passed_time": null, "retry_count": 0},
    {"id": 22, "name": "update-mvcc",        "status": "pending", "passed_time": null, "retry_count": 0},
    {"id": 23, "name": "big-query",          "status": "pending", "passed_time": null, "retry_count": 0},
    {"id": 24, "name": "big-write",          "status": "pending", "passed_time": null, "retry_count": 0},
    {"id": 25, "name": "big-order-by",       "status": "pending", "passed_time": null, "retry_count": 0}
  ],
  "git": {
    "repo": "https://github.com/neellygainforthuj330-bot/miniob_basic",
    "current_branch": "main"
  },
  "submission": {
    "total_attempts": 0,
    "last_submit_time": null,
    "next_available_submit_time": null,
    "last_commit_id": null,
    "last_result": null
  }
}
```

## 状态转换

```
pending → 开发中 → submitted → 等结果 → 分析结果
                                              ↓
                                         passed → 下一题
                                              ↓
                                         failed → pending_retry → 等待冷却(600s) → pending
```

## 异常处理策略

| 异常场景 | 检测方式 | 处理策略 |
|----------|----------|----------|
| 登录失败 | 页面元素未找到 | 重试 3 次，间隔 30 秒 |
| 提交失败 | 返回错误或超时 | 冷却后重试同一题 |
| 编译失败 | build.sh 非 0 | 分析错误后重新 push |
| 网络中断 | 请求超时 | 指数退避：2s→4s→8s→16s |
| 验证码 | 检测到验证码元素 | 标记等待人工介入 |
| Git 冲突 | push 被拒绝 | `git pull --rebase` 后重试 |
| 平台维护 | 页面返回 503 | 每 5 分钟轮询直到恢复 |

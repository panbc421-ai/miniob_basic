# 回归测试套件

## 基线功能与测试用例

| 功能 | 测试用例命令行参数 |
|------|---------------------|
| basic | `--test-cases=basic` |
| AGGREGATION-FUNC | `--test-cases=primary-aggregation-func` |
| DATE | `--test-cases=primary-date` |
| DROP TABLE | `--test-cases=primary-drop-table` |
| LIKE | `--test-cases=primary-like` |
| ORDER-BY | `--test-cases=primary-order-by` |

## 运行命令

```bash
docker exec -w /root/miniob/miniob-2023-09fe80d885b23450fdb01d7b9276ad0f0b6151c0 miniob python test/case/miniob_test.py --test-cases=basic,primary-aggregation-func,primary-date,primary-drop-table,primary-like,primary-order-by
```

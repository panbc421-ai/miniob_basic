"""
analyzer.py — 测试失败结果分析与修复建议生成
"""

PATTERNS = {
    "parser":   (["syntax error", "parse error", "token", "lexical"],
                 "检查词法/语法解析文件：lex_sql.l、yacc_sql.y"),
    "type":     (["type mismatch", "incompatible type", "date"],
                 "检查 Value 类中的类型处理逻辑"),
    "index":    (["bplus tree", "index scan", "comparator"],
                 "检查 bplus_tree 中的比较器实现"),
    "null":     (["null pointer", "null value", "nullable"],
                 "检查 record bitmap 表示和 field_meta nullable 标志"),
    "join":     (["join failed", "cartesian", "on condition"],
                 "检查 Expression 树构建和 join 算子实现"),
    "subquery": (["subquery", "exists", "in clause"],
                 "检查子查询的 Expression 封装和递归处理"),
}


class ResultAnalyzer:

    def analyze(self, test_output: str) -> dict:
        output_lower = test_output.lower()
        matched_patterns = []
        suggestions = []

        for key, (keywords, suggestion) in PATTERNS.items():
            if any(kw in output_lower for kw in keywords):
                matched_patterns.append(key)
                suggestions.append(suggestion)

        if not suggestions:
            suggestions.append("未识别出已知错误模式，请直接查看测试日志")

        return {
            "error_patterns": matched_patterns,
            "fix_suggestions": suggestions,
        }

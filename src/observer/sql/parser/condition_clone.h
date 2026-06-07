#pragma once

#include <memory>
#include <vector>

#include "common/rc.h"
#include "sql/expr/expression.h"
#include "sql/parser/parse_defs.h"

inline std::unique_ptr<SelectSqlNode> clone_select_sql_node(const SelectSqlNode &src);

inline std::unique_ptr<Expression> clone_sql_expression(Expression *expr)
{
  if (expr == nullptr) {
    return nullptr;
  }

  std::unique_ptr<Expression> copy;
  switch (expr->type()) {
    case ExprType::FIELD: {
      auto *field = static_cast<FieldExpr *>(expr);
      copy.reset(new FieldExpr(field->field()));
    } break;
    case ExprType::CORRELATED_FIELD: {
      auto *field = static_cast<CorrelatedFieldExpr *>(expr);
      copy.reset(new CorrelatedFieldExpr(field->table_name(), field->field_name(), field->field_meta()));
    } break;
    case ExprType::UNBOUND_FIELD: {
      auto *field = static_cast<UnboundFieldExpr *>(expr);
      copy.reset(new UnboundFieldExpr(field->table_name(), field->field_name()));
    } break;
    case ExprType::VALUE: {
      Value value;
      if (expr->try_get_value(value) != RC::SUCCESS) {
        return nullptr;
      }
      copy.reset(new ValueExpr(value));
    } break;
    case ExprType::ARITHMETIC: {
      auto *arith = static_cast<ArithmeticExpr *>(expr);
      auto left = clone_sql_expression(arith->left().get());
      std::unique_ptr<Expression> right;
      if (arith->right()) {
        right = clone_sql_expression(arith->right().get());
      }
      if (arith->left() && left == nullptr) {
        return nullptr;
      }
      if (arith->right() && right == nullptr) {
        return nullptr;
      }
      copy.reset(new ArithmeticExpr(arith->arithmetic_type(), std::move(left), std::move(right)));
    } break;
    case ExprType::COMPARISON: {
      auto *cmp = static_cast<ComparisonExpr *>(expr);
      auto left = clone_sql_expression(cmp->left().get());
      auto right = clone_sql_expression(cmp->right().get());
      if ((cmp->left() && left == nullptr) || (cmp->right() && right == nullptr)) {
        return nullptr;
      }
      copy.reset(new ComparisonExpr(cmp->comp(), std::move(left), std::move(right)));
    } break;
    case ExprType::CONJUNCTION: {
      auto *conj = static_cast<ConjunctionExpr *>(expr);
      std::vector<std::unique_ptr<Expression>> children;
      for (auto &child : conj->children()) {
        auto child_copy = clone_sql_expression(child.get());
        if (child != nullptr && child_copy == nullptr) {
          return nullptr;
        }
        children.emplace_back(std::move(child_copy));
      }
      copy.reset(new ConjunctionExpr(conj->conjunction_type(), children));
    } break;
    case ExprType::CAST: {
      auto *cast = static_cast<CastExpr *>(expr);
      auto child = clone_sql_expression(cast->child().get());
      if (cast->child() && child == nullptr) {
        return nullptr;
      }
      copy.reset(new CastExpr(std::move(child), cast->value_type()));
    } break;
    case ExprType::FUNCTION: {
      auto *func = static_cast<FunctionExpr *>(expr);
      std::vector<std::unique_ptr<Expression>> args;
      for (auto &arg : func->args()) {
        auto arg_copy = clone_sql_expression(arg.get());
        if (arg != nullptr && arg_copy == nullptr) {
          return nullptr;
        }
        args.emplace_back(std::move(arg_copy));
      }
      copy.reset(new FunctionExpr(func->func_name(), std::move(args)));
    } break;
    case ExprType::AGGREGATION: {
      auto *agg = static_cast<AggregationExpr *>(expr);
      copy.reset(new AggregationExpr(agg->agg_type(), agg->table_name(), agg->field_name()));
    } break;
    case ExprType::IN_LIST: {
      auto *in_list = static_cast<InListExpr *>(expr);
      std::vector<Value> values = in_list->values();
      copy.reset(new InListExpr(std::move(values)));
    } break;
    case ExprType::SUBQUERY: {
      auto *subquery = dynamic_cast<SubQueryExpr *>(expr);
      if (subquery == nullptr || subquery->select_node() == nullptr) {
        return nullptr;
      }
      auto select_copy = clone_select_sql_node(*subquery->select_node());
      if (select_copy == nullptr) {
        return nullptr;
      }
      copy.reset(new SubQueryExpr(select_copy.release()));
    } break;
    default:
      return nullptr;
  }

  copy->set_name(expr->name());
  return copy;
}

inline RC clone_condition_sql_node(const ConditionSqlNode &src, ConditionSqlNode &dst)
{
  dst.left_is_attr = src.left_is_attr;
  dst.left_value = src.left_value;
  dst.left_attr = src.left_attr;
  dst.comp = src.comp;
  dst.right_is_attr = src.right_is_attr;
  dst.right_attr = src.right_attr;
  dst.right_value = src.right_value;
  dst.connect_or = src.connect_or;

  if (src.left_expr != nullptr) {
    auto left = clone_sql_expression(src.left_expr);
    if (left == nullptr) {
      return RC::INVALID_ARGUMENT;
    }
    dst.left_expr = left.release();
  }
  if (src.right_expr != nullptr) {
    auto right = clone_sql_expression(src.right_expr);
    if (right == nullptr) {
      return RC::INVALID_ARGUMENT;
    }
    dst.right_expr = right.release();
  }
  return RC::SUCCESS;
}

inline RC append_cloned_conditions(const std::vector<ConditionSqlNode> &src, std::vector<ConditionSqlNode> &dst)
{
  for (const ConditionSqlNode &condition : src) {
    ConditionSqlNode copy;
    RC rc = clone_condition_sql_node(condition, copy);
    if (rc != RC::SUCCESS) {
      return rc;
    }
    dst.emplace_back(std::move(copy));
  }
  return RC::SUCCESS;
}

inline RC clone_select_expr_node(const SelectExprNode &src, SelectExprNode &dst)
{
  dst.alias = src.alias;
  dst.agg_type = src.agg_type;
  dst.agg_field = src.agg_field;
  dst.agg_table = src.agg_table;
  dst.is_star = src.is_star;
  dst.star_table = src.star_table;

  if (src.expr != nullptr) {
    auto expr = clone_sql_expression(src.expr);
    if (expr == nullptr) {
      return RC::INVALID_ARGUMENT;
    }
    dst.expr = expr.release();
  }
  return RC::SUCCESS;
}

inline std::unique_ptr<SelectSqlNode> clone_select_sql_node(const SelectSqlNode &src)
{
  std::unique_ptr<SelectSqlNode> dst(new SelectSqlNode);
  dst->attributes = src.attributes;
  dst->relations = src.relations;
  dst->aliases = src.aliases;
  dst->group_by = src.group_by;
  dst->order_by = src.order_by;

  if (append_cloned_conditions(src.conditions, dst->conditions) != RC::SUCCESS ||
      append_cloned_conditions(src.having, dst->having) != RC::SUCCESS) {
    return nullptr;
  }

  for (const SelectExprNode &expr : src.expressions) {
    SelectExprNode copy;
    if (clone_select_expr_node(expr, copy) != RC::SUCCESS) {
      return nullptr;
    }
    dst->expressions.emplace_back(std::move(copy));
  }
  return dst;
}

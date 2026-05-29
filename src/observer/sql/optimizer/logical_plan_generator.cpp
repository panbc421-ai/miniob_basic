/* Copyright (c) 2023 OceanBase and/or its affiliates. All rights reserved.
miniob is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
         http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

//
// Created by Wangyunlai on 2023/08/16.
//

#include "sql/optimizer/logical_plan_generator.h"

#include <set>

#include "sql/operator/logical_operator.h"
#include "sql/operator/calc_logical_operator.h"
#include "sql/operator/project_logical_operator.h"
#include "sql/operator/predicate_logical_operator.h"
#include "sql/operator/table_get_logical_operator.h"
#include "sql/operator/insert_logical_operator.h"
#include "sql/operator/delete_logical_operator.h"
#include "sql/operator/join_logical_operator.h"
#include "sql/operator/project_logical_operator.h"
#include "sql/operator/explain_logical_operator.h"

#include "sql/expr/expression.h"
#include "sql/stmt/stmt.h"
#include "sql/stmt/calc_stmt.h"
#include "sql/stmt/select_stmt.h"
#include "sql/stmt/filter_stmt.h"
#include "sql/stmt/insert_stmt.h"
#include "sql/stmt/delete_stmt.h"
#include "sql/stmt/explain_stmt.h"

using namespace std;

// 从表达式树中收集所有引用的表名
static void collect_table_refs(Expression *expr, std::set<std::string> &tables)
{
  if (expr == nullptr) return;
  switch (expr->type()) {
    case ExprType::FIELD: {
      auto *fe = static_cast<FieldExpr *>(expr);
      tables.insert(fe->table_name());
    } break;
    case ExprType::ARITHMETIC: {
      auto *ae = static_cast<ArithmeticExpr *>(expr);
      if (ae->left()) collect_table_refs(ae->left().get(), tables);
      if (ae->right()) collect_table_refs(ae->right().get(), tables);
    } break;
    case ExprType::CAST: {
      auto *ce = static_cast<CastExpr *>(expr);
      collect_table_refs(ce->child().get(), tables);
    } break;
    case ExprType::COMPARISON: {
      auto *cmp = static_cast<ComparisonExpr *>(expr);
      if (cmp->left()) collect_table_refs(cmp->left().get(), tables);
      if (cmp->right()) collect_table_refs(cmp->right().get(), tables);
    } break;
    case ExprType::CONJUNCTION: {
      auto *conj = static_cast<ConjunctionExpr *>(expr);
      for (auto &child : conj->children()) {
        collect_table_refs(child.get(), tables);
      }
    } break;
    default:
      break;
  }
}

// 深克隆一个表达式树（用于将已解析的表达式从 FilterUnit 复制到 ComparisonExpr）
static std::unique_ptr<Expression> clone_expr_tree(Expression *e)
{
  if (e == nullptr) return nullptr;
  switch (e->type()) {
    case ExprType::FIELD: {
      auto *fe = static_cast<FieldExpr *>(e);
      return std::unique_ptr<Expression>(new FieldExpr(fe->field()));
    }
    case ExprType::VALUE: {
      Value v;
      e->try_get_value(v);
      return std::unique_ptr<Expression>(new ValueExpr(v));
    }
    case ExprType::ARITHMETIC: {
      auto *ae = static_cast<ArithmeticExpr *>(e);
      auto left = clone_expr_tree(ae->left().get());
      auto right = clone_expr_tree(ae->right().get());
      return std::unique_ptr<Expression>(
          new ArithmeticExpr(ae->arithmetic_type(), std::move(left), std::move(right)));
    }
    case ExprType::CAST: {
      auto *ce = static_cast<CastExpr *>(e);
      auto child = clone_expr_tree(ce->child().get());
      return std::unique_ptr<Expression>(new CastExpr(std::move(child), ce->value_type()));
    }
    default:
      return nullptr;
  }
}

RC LogicalPlanGenerator::create(Stmt *stmt, unique_ptr<LogicalOperator> &logical_operator)
{
  RC rc = RC::SUCCESS;
  switch (stmt->type()) {
    case StmtType::CALC: {
      CalcStmt *calc_stmt = static_cast<CalcStmt *>(stmt);
      rc = create_plan(calc_stmt, logical_operator);
    } break;

    case StmtType::SELECT: {
      SelectStmt *select_stmt = static_cast<SelectStmt *>(stmt);
      rc = create_plan(select_stmt, logical_operator);
    } break;

    case StmtType::INSERT: {
      InsertStmt *insert_stmt = static_cast<InsertStmt *>(stmt);
      rc = create_plan(insert_stmt, logical_operator);
    } break;

    case StmtType::DELETE: {
      DeleteStmt *delete_stmt = static_cast<DeleteStmt *>(stmt);
      rc = create_plan(delete_stmt, logical_operator);
    } break;

    case StmtType::EXPLAIN: {
      ExplainStmt *explain_stmt = static_cast<ExplainStmt *>(stmt);
      rc = create_plan(explain_stmt, logical_operator);
    } break;
    default: {
      rc = RC::UNIMPLENMENT;
    }
  }
  return rc;
}

RC LogicalPlanGenerator::create_plan(CalcStmt *calc_stmt, std::unique_ptr<LogicalOperator> &logical_operator)
{
  logical_operator.reset(new CalcLogicalOperator(std::move(calc_stmt->expressions())));
  return RC::SUCCESS;
}

RC LogicalPlanGenerator::create_plan(
    SelectStmt *select_stmt, unique_ptr<LogicalOperator> &logical_operator)
{
  unique_ptr<LogicalOperator> table_oper(nullptr);

  const std::vector<Table *> &tables = select_stmt->tables();
  const std::vector<Field> &all_fields = select_stmt->query_fields();

  // Partition filter units: per-table vs join-level.
  // Also track which tables each join filter references for interleaving.
  FilterStmt *filter_stmt = select_stmt->filter_stmt();
  std::vector<const FilterUnit *> join_filters;
  std::vector<std::set<std::string>> join_filter_tables;
  std::unordered_map<std::string, std::vector<const FilterUnit *>> table_filters;
  std::vector<const FilterUnit *> expr_filters;  // 基于表达式的filter

  if (filter_stmt != nullptr) {
    for (const FilterUnit *unit : filter_stmt->filter_units()) {
      // 基于表达式的 filter: 尝试分类以支持 join interleaving
      if (unit->is_expr_based()) {
        std::set<std::string> ref_tables;
        if (unit->left_expr())  collect_table_refs(unit->left_expr(), ref_tables);
        if (unit->right_expr()) collect_table_refs(unit->right_expr(), ref_tables);
        if (ref_tables.empty()) {
          // 常量比较（如 1=1）：跳过，稍后在 expr_filters 中处理
          expr_filters.push_back(unit);
        } else if (ref_tables.size() == 1) {
          // 单表过滤器
          std::string tn = *ref_tables.begin();
          table_filters[tn].push_back(unit);
        } else {
          // 多表 join 过滤器
          join_filters.push_back(unit);
          join_filter_tables.push_back(ref_tables);
        }
        continue;
      }
      std::string ref_table;
      bool single_table = true;
      std::set<std::string> ref_tables;
      auto check_obj = [&](const FilterObj &obj) {
        if (obj.is_attr) {
          const char *tn = obj.field.table_name();
          ref_tables.insert(tn);
          if (ref_table.empty()) {
            ref_table = tn;
          } else if (ref_table != std::string(tn)) {
            single_table = false;
          }
        }
      };
      check_obj(unit->left());
      check_obj(unit->right());
      if (ref_table.empty()) {
        // Both sides are constant values (e.g. 1=1): skip, tautology
      } else if (single_table) {
        table_filters[ref_table].push_back(unit);
      } else {
        join_filters.push_back(unit);
        join_filter_tables.push_back(ref_tables);
      }
    }
  }

  std::set<std::string> joined_tables;

  // 当使用表达式select且没有传统字段时，需要扫描所有字段以便表达式求值
  bool need_all_table_fields = all_fields.empty() && !select_stmt->select_exprs().empty();

  for (Table *table : tables) {
    std::vector<Field> fields;
    if (need_all_table_fields) {
      const TableMeta &table_meta = table->table_meta();
      for (int i = table_meta.sys_field_num(); i < table_meta.field_num(); i++) {
        fields.push_back(Field(table, table_meta.field(i)));
      }
    } else {
      for (const Field &field : all_fields) {
        if (0 == strcmp(field.table_name(), table->name())) {
          fields.push_back(field);
        }
      }
    }

    unique_ptr<LogicalOperator> table_get_oper(new TableGetLogicalOperator(table, fields, true/*readonly*/));

    // Wrap with per-table predicate if applicable
    auto it = table_filters.find(table->name());
    if (it != table_filters.end() && !it->second.empty()) {
      std::vector<unique_ptr<Expression>> cmp_exprs;
      for (const FilterUnit *unit : it->second) {
        if (unit->is_expr_based()) {
          auto left  = clone_expr_tree(unit->left_expr());
          auto right = clone_expr_tree(unit->right_expr());
          cmp_exprs.emplace_back(new ComparisonExpr(unit->comp(), std::move(left), std::move(right)));
        } else {
          auto make_expr = [](const FilterObj &obj) -> unique_ptr<Expression> {
            if (obj.is_attr) return unique_ptr<Expression>(new FieldExpr(obj.field));
            else return unique_ptr<Expression>(new ValueExpr(obj.value));
          };
          cmp_exprs.emplace_back(new ComparisonExpr(unit->comp(), make_expr(unit->left()), make_expr(unit->right())));
        }
      }
      unique_ptr<ConjunctionExpr> conj(new ConjunctionExpr(ConjunctionExpr::Type::AND, cmp_exprs));
      unique_ptr<PredicateLogicalOperator> pred(new PredicateLogicalOperator(std::move(conj)));
      pred->add_child(std::move(table_get_oper));
      table_get_oper = std::move(pred);
    }

    std::string table_name = table->name();

    if (table_oper == nullptr) {
      table_oper = std::move(table_get_oper);
    } else {
      JoinLogicalOperator *join_oper = new JoinLogicalOperator;
      join_oper->add_child(std::move(table_oper));
      join_oper->add_child(std::move(table_get_oper));
      table_oper = unique_ptr<LogicalOperator>(join_oper);
    }

    joined_tables.insert(table_name);

    // Interleave join predicates: pull out filters whose tables are all now joined
    if (!join_filters.empty()) {
      std::vector<const FilterUnit *> covered;
      auto fit = join_filters.begin();
      auto tit = join_filter_tables.begin();
      while (fit != join_filters.end()) {
        bool all_joined = true;
        for (const std::string &t : *tit) {
          if (joined_tables.find(t) == joined_tables.end()) {
            all_joined = false;
            break;
          }
        }
        if (all_joined) {
          covered.push_back(*fit);
          fit = join_filters.erase(fit);
          tit = join_filter_tables.erase(tit);
        } else {
          ++fit;
          ++tit;
        }
      }

      if (!covered.empty()) {
        std::vector<unique_ptr<Expression>> cmp_exprs;
        for (const FilterUnit *unit : covered) {
          if (unit->is_expr_based()) {
            auto left  = clone_expr_tree(unit->left_expr());
            auto right = clone_expr_tree(unit->right_expr());
            cmp_exprs.emplace_back(new ComparisonExpr(unit->comp(), std::move(left), std::move(right)));
          } else {
            auto make_expr = [](const FilterObj &obj) -> unique_ptr<Expression> {
              if (obj.is_attr) return unique_ptr<Expression>(new FieldExpr(obj.field));
              else return unique_ptr<Expression>(new ValueExpr(obj.value));
            };
            cmp_exprs.emplace_back(new ComparisonExpr(unit->comp(), make_expr(unit->left()), make_expr(unit->right())));
          }
        }
        unique_ptr<ConjunctionExpr> conj(new ConjunctionExpr(ConjunctionExpr::Type::AND, cmp_exprs));
        unique_ptr<PredicateLogicalOperator> pred(new PredicateLogicalOperator(std::move(conj)));
        pred->add_child(std::move(table_oper));
        table_oper = std::move(pred);
      }
    }
  }

  // Create join-level predicate from remaining filters (legacy + expression-based)
  unique_ptr<LogicalOperator> predicate_oper;
  if (!join_filters.empty() || !expr_filters.empty()) {
    std::vector<unique_ptr<Expression>> cmp_exprs;
    // Legacy join filters (FilterObj based)
    for (const FilterUnit *unit : join_filters) {
      auto make_expr = [](const FilterObj &obj) -> unique_ptr<Expression> {
        if (obj.is_attr) {
          return unique_ptr<Expression>(new FieldExpr(obj.field));
        } else {
          return unique_ptr<Expression>(new ValueExpr(obj.value));
        }
      };
      ComparisonExpr *cmp = new ComparisonExpr(unit->comp(), make_expr(unit->left()), make_expr(unit->right()));
      cmp_exprs.emplace_back(cmp);
    }
    // Expression-based filters (clone the resolved expression trees)
    for (const FilterUnit *unit : expr_filters) {
      auto left = clone_expr_tree(unit->left_expr());
      auto right = clone_expr_tree(unit->right_expr());
      ComparisonExpr *cmp = new ComparisonExpr(unit->comp(), std::move(left), std::move(right));
      cmp_exprs.emplace_back(cmp);
    }
    unique_ptr<ConjunctionExpr> conj(new ConjunctionExpr(ConjunctionExpr::Type::AND, cmp_exprs));
    predicate_oper = unique_ptr<PredicateLogicalOperator>(new PredicateLogicalOperator(std::move(conj)));
  }

  unique_ptr<LogicalOperator> project_oper(new ProjectLogicalOperator(all_fields));

  // 将基于表达式的select items转移到project算子
  auto &select_exprs_ref = select_stmt->select_exprs();
  if (!select_exprs_ref.empty()) {
    auto &proj_exprs = static_cast<ProjectLogicalOperator *>(project_oper.get())->expressions();
    // 先将所有 field-based select items 转换为 FieldExpr
    for (const auto &f : all_fields) {
      proj_exprs.emplace_back(new FieldExpr(f));
    }
    // 再追加复杂表达式
    for (auto &se : select_exprs_ref) {
      if (se.expr) {
        proj_exprs.emplace_back(se.expr);
        se.expr = nullptr;
      }
    }
  }

  if (predicate_oper) {
    if (table_oper) {
      predicate_oper->add_child(std::move(table_oper));
    }
    project_oper->add_child(std::move(predicate_oper));
  } else {
    if (table_oper) {
      project_oper->add_child(std::move(table_oper));
    }
  }

  logical_operator.swap(project_oper);
  return RC::SUCCESS;
}

RC LogicalPlanGenerator::create_plan(
    FilterStmt *filter_stmt, unique_ptr<LogicalOperator> &logical_operator)
{
  std::vector<unique_ptr<Expression>> cmp_exprs;
  const std::vector<FilterUnit *> &filter_units = filter_stmt->filter_units();
  for (const FilterUnit *filter_unit : filter_units) {
    const FilterObj &filter_obj_left = filter_unit->left();
    const FilterObj &filter_obj_right = filter_unit->right();

    unique_ptr<Expression> left(filter_obj_left.is_attr
                                         ? static_cast<Expression *>(new FieldExpr(filter_obj_left.field))
                                         : static_cast<Expression *>(new ValueExpr(filter_obj_left.value)));

    unique_ptr<Expression> right(filter_obj_right.is_attr
                                          ? static_cast<Expression *>(new FieldExpr(filter_obj_right.field))
                                          : static_cast<Expression *>(new ValueExpr(filter_obj_right.value)));

    ComparisonExpr *cmp_expr = new ComparisonExpr(filter_unit->comp(), std::move(left), std::move(right));
    cmp_exprs.emplace_back(cmp_expr);
  }

  unique_ptr<PredicateLogicalOperator> predicate_oper;
  if (!cmp_exprs.empty()) {
    unique_ptr<ConjunctionExpr> conjunction_expr(new ConjunctionExpr(ConjunctionExpr::Type::AND, cmp_exprs));
    predicate_oper = unique_ptr<PredicateLogicalOperator>(new PredicateLogicalOperator(std::move(conjunction_expr)));
  }

  logical_operator = std::move(predicate_oper);
  return RC::SUCCESS;
}

RC LogicalPlanGenerator::create_plan(
    InsertStmt *insert_stmt, unique_ptr<LogicalOperator> &logical_operator)
{
  Table *table = insert_stmt->table();
  vector<Value> values(insert_stmt->values(), insert_stmt->values() + insert_stmt->value_amount());

  InsertLogicalOperator *insert_operator = new InsertLogicalOperator(table, values);
  logical_operator.reset(insert_operator);
  return RC::SUCCESS;
}

RC LogicalPlanGenerator::create_plan(
    DeleteStmt *delete_stmt, unique_ptr<LogicalOperator> &logical_operator)
{
  Table *table = delete_stmt->table();
  FilterStmt *filter_stmt = delete_stmt->filter_stmt();
  std::vector<Field> fields;
  for (int i = table->table_meta().sys_field_num(); i < table->table_meta().field_num(); i++) {
    const FieldMeta *field_meta = table->table_meta().field(i);
    fields.push_back(Field(table, field_meta));
  }
  unique_ptr<LogicalOperator> table_get_oper(new TableGetLogicalOperator(table, fields, false/*readonly*/));

  unique_ptr<LogicalOperator> predicate_oper;
  RC rc = create_plan(filter_stmt, predicate_oper);
  if (rc != RC::SUCCESS) {
    return rc;
  }

  unique_ptr<LogicalOperator> delete_oper(new DeleteLogicalOperator(table));

  if (predicate_oper) {
    predicate_oper->add_child(std::move(table_get_oper));
    delete_oper->add_child(std::move(predicate_oper));
  } else {
    delete_oper->add_child(std::move(table_get_oper));
  }

  logical_operator = std::move(delete_oper);
  return rc;
}

RC LogicalPlanGenerator::create_plan(
    ExplainStmt *explain_stmt, unique_ptr<LogicalOperator> &logical_operator)
{
  Stmt *child_stmt = explain_stmt->child();
  unique_ptr<LogicalOperator> child_oper;
  RC rc = create(child_stmt, child_oper);
  if (rc != RC::SUCCESS) {
    LOG_WARN("failed to create explain's child operator. rc=%s", strrc(rc));
    return rc;
  }

  logical_operator = unique_ptr<LogicalOperator>(new ExplainLogicalOperator);
  logical_operator->add_child(std::move(child_oper));
  return rc;
}

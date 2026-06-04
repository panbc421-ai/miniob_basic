/* Copyright (c) 2021 OceanBase and/or its affiliates. All rights reserved.
miniob is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
         http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

#include "common/rc.h"
#include "common/log/log.h"
#include "common/lang/string.h"
#include "sql/stmt/filter_stmt.h"
#include "sql/stmt/select_stmt.h"
#include "sql/optimizer/logical_plan_generator.h"
#include "sql/optimizer/physical_plan_generator.h"
#include "sql/operator/aggregation_physical_operator.h"
#include "storage/db/db.h"
#include "storage/table/table.h"
#include "session/session.h"

#include <memory>
#include <set>

// Collect inner-query table / alias names from parsed SELECT (read-only).
static void collect_inner_table_names(const SelectSqlNode &sel, std::set<std::string> &inner)
{
  for (size_t i = 0; i < sel.relations.size(); i++) {
    inner.insert(sel.relations[i]);
    if (i < sel.aliases.size() && !sel.aliases[i].empty()) {
      inner.insert(sel.aliases[i]);
    }
  }
}

// Whether an unresolved expression references a table from the outer scope.
static bool expr_refs_outer_scope(Expression *expr, const std::set<std::string> &inner,
    const std::unordered_map<std::string, Table *> *outer_tables)
{
  if (expr == nullptr || outer_tables == nullptr) {
    return false;
  }
  switch (expr->type()) {
    case ExprType::UNBOUND_FIELD: {
      const auto *uf = static_cast<UnboundFieldExpr *>(expr);
      const std::string &tn = uf->table_name();
      if (tn.empty()) {
        return false;
      }
      return inner.find(tn) == inner.end() && outer_tables->find(tn) != outer_tables->end();
    }
    case ExprType::CORRELATED_FIELD:
      return true;
    case ExprType::ARITHMETIC: {
      auto *ae = static_cast<ArithmeticExpr *>(expr);
      return expr_refs_outer_scope(ae->left().get(), inner, outer_tables) ||
             (ae->right() != nullptr && expr_refs_outer_scope(ae->right().get(), inner, outer_tables));
    }
    case ExprType::COMPARISON: {
      auto *ce = static_cast<ComparisonExpr *>(expr);
      return expr_refs_outer_scope(ce->left().get(), inner, outer_tables) ||
             expr_refs_outer_scope(ce->right().get(), inner, outer_tables);
    }
    case ExprType::CONJUNCTION: {
      auto *cj = static_cast<ConjunctionExpr *>(expr);
      for (auto &c : cj->children()) {
        if (expr_refs_outer_scope(c.get(), inner, outer_tables)) {
          return true;
        }
      }
      return false;
    }
    case ExprType::CAST: {
      auto *ct = static_cast<CastExpr *>(expr);
      return expr_refs_outer_scope(ct->child().get(), inner, outer_tables);
    }
    case ExprType::FUNCTION: {
      auto *fn = static_cast<FunctionExpr *>(expr);
      for (auto &a : fn->args()) {
        if (expr_refs_outer_scope(a.get(), inner, outer_tables)) {
          return true;
        }
      }
      return false;
    }
    default:
      return false;
  }
}

// Detect correlation without building Stmt (SelectStmt::create mutates SelectSqlNode).
static bool select_node_has_correlation(const SelectSqlNode &sel,
    const std::unordered_map<std::string, Table *> *outer_tables)
{
  if (outer_tables == nullptr || outer_tables->empty()) {
    return false;
  }
  std::set<std::string> inner;
  collect_inner_table_names(sel, inner);
  for (const ConditionSqlNode &cond : sel.conditions) {
    if (expr_refs_outer_scope(cond.left_expr, inner, outer_tables) ||
        expr_refs_outer_scope(cond.right_expr, inner, outer_tables)) {
      return true;
    }
  }
  return false;
}

// Deep-clone parse-stage expression trees (before SelectStmt::create mutates them).
static std::unique_ptr<Expression> clone_parse_expression(Expression *expr)
{
  if (expr == nullptr) {
    return nullptr;
  }
  switch (expr->type()) {
    case ExprType::UNBOUND_FIELD: {
      auto *uf = static_cast<UnboundFieldExpr *>(expr);
      auto *copy = new UnboundFieldExpr(uf->table_name(), uf->field_name());
      copy->set_name(expr->name());
      return std::unique_ptr<Expression>(copy);
    }
    case ExprType::VALUE: {
      Value v;
      expr->try_get_value(v);
      auto *copy = new ValueExpr(v);
      copy->set_name(expr->name());
      return std::unique_ptr<Expression>(copy);
    }
    case ExprType::ARITHMETIC: {
      auto *ae = static_cast<ArithmeticExpr *>(expr);
      auto left = clone_parse_expression(ae->left().get());
      std::unique_ptr<Expression> right;
      if (ae->right()) {
        right = clone_parse_expression(ae->right().get());
      }
      auto *copy = new ArithmeticExpr(ae->arithmetic_type(), std::move(left), std::move(right));
      copy->set_name(expr->name());
      return std::unique_ptr<Expression>(copy);
    }
    case ExprType::COMPARISON: {
      auto *ce = static_cast<ComparisonExpr *>(expr);
      auto left = clone_parse_expression(ce->left().get());
      auto right = clone_parse_expression(ce->right().get());
      auto *copy = new ComparisonExpr(ce->comp(), std::move(left), std::move(right));
      copy->set_name(expr->name());
      return std::unique_ptr<Expression>(copy);
    }
    case ExprType::CONJUNCTION: {
      auto *cj = static_cast<ConjunctionExpr *>(expr);
      std::vector<std::unique_ptr<Expression>> children;
      for (auto &child : cj->children()) {
        children.emplace_back(clone_parse_expression(child.get()));
      }
      auto *copy = new ConjunctionExpr(cj->conjunction_type(), children);
      copy->set_name(expr->name());
      return std::unique_ptr<Expression>(copy);
    }
    case ExprType::CAST: {
      auto *ct = static_cast<CastExpr *>(expr);
      auto child = clone_parse_expression(ct->child().get());
      auto *copy = new CastExpr(std::move(child), ct->value_type());
      copy->set_name(expr->name());
      return std::unique_ptr<Expression>(copy);
    }
    case ExprType::FUNCTION: {
      auto *fn = static_cast<FunctionExpr *>(expr);
      std::vector<std::unique_ptr<Expression>> args;
      for (auto &arg : fn->args()) {
        args.emplace_back(clone_parse_expression(arg.get()));
      }
      auto *copy = new FunctionExpr(fn->func_name(), std::move(args));
      copy->set_name(expr->name());
      return std::unique_ptr<Expression>(copy);
    }
    default:
      return nullptr;
  }
}

static SelectSqlNode clone_select_sql_node(const SelectSqlNode &src)
{
  SelectSqlNode dst;
  dst.relations = src.relations;
  dst.aliases = src.aliases;
  dst.attributes = src.attributes;
  dst.group_by = src.group_by;
  dst.order_by = src.order_by;

  for (const ConditionSqlNode &cond : src.conditions) {
    ConditionSqlNode copy;
    copy.left_is_attr = cond.left_is_attr;
    copy.left_attr = cond.left_attr;
    copy.left_value = cond.left_value;
    copy.comp = cond.comp;
    copy.right_is_attr = cond.right_is_attr;
    copy.right_attr = cond.right_attr;
    copy.right_value = cond.right_value;
    if (cond.left_expr != nullptr) {
      copy.left_expr = clone_parse_expression(cond.left_expr).release();
    }
    if (cond.right_expr != nullptr) {
      copy.right_expr = clone_parse_expression(cond.right_expr).release();
    }
    dst.conditions.emplace_back(std::move(copy));
  }

  for (const SelectExprNode &item : src.expressions) {
    SelectExprNode copy;
    copy.alias = item.alias;
    copy.agg_type = item.agg_type;
    copy.agg_field = item.agg_field;
    copy.agg_table = item.agg_table;
    if (item.expr != nullptr) {
      copy.expr = clone_parse_expression(item.expr).release();
    }
    dst.expressions.emplace_back(std::move(copy));
  }
  return std::move(dst);
}

FilterStmt::~FilterStmt()
{
  for (FilterUnit *unit : filter_units_) {
    delete unit;
  }
  filter_units_.clear();
}

RC FilterStmt::create(Db *db, Table *default_table, std::unordered_map<std::string, Table *> *tables,
    const ConditionSqlNode *conditions, int condition_num, FilterStmt *&stmt,
    std::unordered_map<std::string, Table *> *outer_table_map)
{
  RC rc = RC::SUCCESS;
  stmt = nullptr;

  FilterStmt *tmp_stmt = new FilterStmt();
  for (int i = 0; i < condition_num; i++) {
    FilterUnit *filter_unit = nullptr;
    rc = create_filter_unit(db, default_table, tables, conditions[i], filter_unit, outer_table_map);
    if (rc != RC::SUCCESS) {
      delete tmp_stmt;
      LOG_WARN("failed to create filter unit. condition index=%d", i);
      return rc;
    }
    tmp_stmt->filter_units_.push_back(filter_unit);
  }

  stmt = tmp_stmt;
  return rc;
}

RC get_table_and_field(Db *db, Table *default_table, std::unordered_map<std::string, Table *> *tables,
    const RelAttrSqlNode &attr, Table *&table, const FieldMeta *&field,
    std::unordered_map<std::string, Table *> *outer_tables = nullptr)
{
  if (common::is_blank(attr.relation_name.c_str())) {
    table = default_table;
  } else if (nullptr != tables) {
    auto iter = tables->find(attr.relation_name);
    if (iter != tables->end()) {
      table = iter->second;
    }
  } else {
    table = db->find_table(attr.relation_name.c_str());
  }
  // Fallback to outer table_map for correlated subqueries
  if (nullptr == table && outer_tables != nullptr) {
    auto iter = outer_tables->find(attr.relation_name);
    if (iter != outer_tables->end()) {
      table = iter->second;
    }
  }
  if (nullptr == table) {
    LOG_WARN("No such table: attr.relation_name: %s", attr.relation_name.c_str());
    return RC::SCHEMA_TABLE_NOT_EXIST;
  }

  field = table->table_meta().field(attr.attribute_name.c_str());
  if (nullptr == field) {
    LOG_WARN("no such field in table: table %s, field %s", table->name(), attr.attribute_name.c_str());
    table = nullptr;
    return RC::SCHEMA_FIELD_NOT_EXIST;
  }
  return RC::SUCCESS;
}

// 解析一个表达式侧（left 或 right），如果结果是简单类型则转换为 FilterObj
static RC resolve_side(Expression *&expr, Table *default_table,
                       std::unordered_map<std::string, Table *> *tables,
                       FilterObj &obj, bool &is_simple,
                       std::unordered_map<std::string, Table *> *outer_table_map = nullptr)
{
  if (expr == nullptr) return RC::SUCCESS;
  std::unique_ptr<Expression> e(expr);
  expr = nullptr; // 转移所有权
  RC rc = resolve_expression(e, default_table, tables, outer_table_map);
  if (rc != RC::SUCCESS) return rc;
  if (e->type() == ExprType::FIELD) {
    obj.init_attr(static_cast<FieldExpr *>(e.get())->field());
    is_simple = true;
    return RC::SUCCESS;
  } else if (e->type() == ExprType::VALUE) {
    Value v;
    e->try_get_value(v);
    obj.init_value(v);
    is_simple = true;
    return RC::SUCCESS;
  }
  // 复杂表达式: 放回 expr 指针，稍后存入 FilterUnit
  is_simple = false;
  expr = e.release();
  return RC::SUCCESS;
}

// Helper: check if an expression tree contains any CorrelatedFieldExpr
static bool has_correlated_field(Expression *e) {
  if (e == nullptr) return false;
  if (e->type() == ExprType::CORRELATED_FIELD) return true;
  if (e->type() == ExprType::ARITHMETIC) {
    auto *ae = static_cast<ArithmeticExpr *>(e);
    return has_correlated_field(ae->left().get()) || has_correlated_field(ae->right().get());
  }
  if (e->type() == ExprType::COMPARISON) {
    auto *ce = static_cast<ComparisonExpr *>(e);
    return has_correlated_field(ce->left().get()) || has_correlated_field(ce->right().get());
  }
  if (e->type() == ExprType::CONJUNCTION) {
    auto *cj = static_cast<ConjunctionExpr *>(e);
    for (auto &c : cj->children()) {
      if (has_correlated_field(c.get())) return true;
    }
  }
  if (e->type() == ExprType::CAST) {
    auto *ct = static_cast<CastExpr *>(e);
    return has_correlated_field(ct->child().get());
  }
  if (e->type() == ExprType::FUNCTION) {
    auto *fn = static_cast<FunctionExpr *>(e);
    for (auto &a : fn->args()) {
      if (has_correlated_field(a.get())) return true;
    }
  }
  return false;
}

// Lightweight expression wrapper that delegates to a std::function
namespace {
class LambdaExpr : public Expression {
public:
  using Func = std::function<RC(const Tuple &, Value &)>;
  LambdaExpr(Func f) : func_(std::move(f)) {}
  ExprType type() const override { return ExprType::SUBQUERY; }
  AttrType value_type() const override { return BOOLEANS; }
  RC get_value(const Tuple &t, Value &v) const override { return func_(t, v); }
private:
  Func func_;
};
}

RC FilterStmt::create_filter_unit(Db *db, Table *default_table, std::unordered_map<std::string, Table *> *tables,
    const ConditionSqlNode &condition, FilterUnit *&filter_unit,
    std::unordered_map<std::string, Table *> *outer_table_map)
{
  RC rc = RC::SUCCESS;

  CompOp comp = condition.comp;
if (comp < EQUAL_TO || comp >= NO_OP) {
    LOG_WARN("invalid compare operator : %d", comp);
    return RC::INVALID_ARGUMENT;
  }

  // 检查是否有表达式操作数
  bool has_expr = (condition.left_expr != nullptr || condition.right_expr != nullptr);

  if (!has_expr) {
    // === 纯 legacy 路径 ===
    filter_unit = new FilterUnit;
    filter_unit->set_comp(comp);
    if (condition.left_is_attr) {
      Table *table = nullptr;
      const FieldMeta *field = nullptr;
      rc = get_table_and_field(db, default_table, tables, condition.left_attr, table, field, outer_table_map);
      if (rc != RC::SUCCESS) { delete filter_unit; return rc; }
      FilterObj obj; obj.init_attr(Field(table, field));
      filter_unit->set_left(obj);
    } else {
      FilterObj obj; obj.init_value(condition.left_value);
      filter_unit->set_left(obj);
    }
    if (condition.right_is_attr) {
      Table *table = nullptr;
      const FieldMeta *field = nullptr;
      rc = get_table_and_field(db, default_table, tables, condition.right_attr, table, field, outer_table_map);
      if (rc != RC::SUCCESS) { delete filter_unit; return rc; }
      FilterObj obj; obj.init_attr(Field(table, field));
      filter_unit->set_right(obj);
    } else {
      FilterObj obj; obj.init_value(condition.right_value);
      filter_unit->set_right(obj);
    }
  } else {
    // === 表达式路径：解析两边，尝试简化为 legacy ===
    FilterObj left_obj, right_obj;
    bool left_simple = false, right_simple = false;

    // 转移并解析 left
    Expression *left_e = condition.left_expr;
    if (left_e) const_cast<ConditionSqlNode &>(condition).left_expr = nullptr;
    rc = resolve_side(left_e, default_table, tables, left_obj, left_simple, outer_table_map);
    if (rc != RC::SUCCESS) return rc;

    // 转移并解析 right
    Expression *right_e = condition.right_expr;
    if (right_e) const_cast<ConditionSqlNode &>(condition).right_expr = nullptr;
    rc = resolve_side(right_e, default_table, tables, right_obj, right_simple, outer_table_map);
    if (rc != RC::SUCCESS) { delete left_e; return rc; }

    // Check for subquery in right side
    bool right_is_subquery = (right_e != nullptr && right_e->type() == ExprType::SUBQUERY);
    bool left_is_subquery  = (left_e  != nullptr && left_e->type()  == ExprType::SUBQUERY);
    bool is_in_op    = (comp == IN_OP || comp == NOT_IN);
    bool is_exists_op = (comp == EXISTS_OP || comp == NOT_EXISTS);

    if (right_is_subquery || left_is_subquery) {
      auto *sq = static_cast<SubQueryExpr *>(right_is_subquery ? right_e : left_e);
      auto *other_expr = right_is_subquery ? left_e : right_e;

      // Detect correlation on the parse tree only. Do NOT call SelectStmt::create here:
      // it consumes/moves condition expressions and would break per-row re-execution.
      std::unordered_map<std::string, Table *> merged_outer;
      if (outer_table_map != nullptr) {
        merged_outer.insert(outer_table_map->begin(), outer_table_map->end());
      }
      if (tables != nullptr) {
        for (const auto &kv : *tables) {
          merged_outer[kv.first] = kv.second;
        }
      }
      const std::unordered_map<std::string, Table *> *detect_outer =
          merged_outer.empty() ? nullptr : &merged_outer;
      bool is_correlated = select_node_has_correlation(*sq->select_node(), detect_outer);

      Stmt *inner_stmt = nullptr;
      if (!is_correlated) {
        rc = SelectStmt::create(db, *sq->select_node(), inner_stmt, tables);
        if (rc == RC::SCHEMA_TABLE_NOT_EXIST && outer_table_map != nullptr) {
          rc = SelectStmt::create(db, *sq->select_node(), inner_stmt, outer_table_map);
        }
      }

      if (is_correlated) {
        // === Correlated subquery: per-row execution ===
        // Transfer ownership of the SubQueryExpr to keep the SelectSqlNode alive
        std::shared_ptr<Expression> subquery_holder(right_is_subquery ? right_e : left_e);
        if (right_is_subquery) right_e = nullptr; else left_e = nullptr;

        // Outer context for correlated fields: current query level (with aliases)
        // plus any further-out scopes.
        std::shared_ptr<std::unordered_map<std::string, Table *>> captured_outer_map =
            std::make_shared<std::unordered_map<std::string, Table *>>();
        if (outer_table_map != nullptr) {
          captured_outer_map->insert(outer_table_map->begin(), outer_table_map->end());
        }
        if (tables != nullptr) {
          for (const auto &kv : *tables) {
            (*captured_outer_map)[kv.first] = kv.second;
          }
        }

        // Capture the other side expression for IN/NOT IN comparison
        std::shared_ptr<Expression> captured_other;
        if (other_expr != nullptr) {
          captured_other.reset(other_expr);
          if (right_is_subquery) left_e = nullptr; else right_e = nullptr;
        }
        // Use the FilterObj from resolution if the other side was simple
        FilterObj other_obj = right_is_subquery ? left_obj : right_obj;

        // Keep an immutable snapshot; SelectStmt::create mutates the parse tree.
        auto select_template =
            std::make_shared<SelectSqlNode>(clone_select_sql_node(*sq->select_node()));

        auto eval_func =
            [db, select_template, sq_holder = subquery_holder, outer_map = captured_outer_map,
             comp, is_in_op, is_exists_op, right_is_subquery, other_obj,
             other_expr_ptr = captured_other](
                const Tuple &outer_tuple, Value &value) mutable -> RC {
          // Set the outer tuple for CorrelatedFieldExpr evaluation
          g_outer_tuple = &outer_tuple;

          SelectSqlNode sel_copy = clone_select_sql_node(*select_template);
          Stmt *inner = nullptr;
          RC local_rc = SelectStmt::create(db, sel_copy, inner, outer_map.get());
          if (local_rc != RC::SUCCESS) {
            g_outer_tuple = nullptr;
            value.set_boolean(false);
            return local_rc;
          }
          auto *sel_stmt = static_cast<SelectStmt *>(inner);

          LogicalPlanGenerator logic_gen;
          std::unique_ptr<LogicalOperator> logic_oper;
          local_rc = logic_gen.create(sel_stmt, logic_oper);
          if (local_rc != RC::SUCCESS) {
            g_outer_tuple = nullptr;
            delete sel_stmt;
            value.set_boolean(false);
            return local_rc;
          }

          PhysicalPlanGenerator phys_gen;
          std::unique_ptr<PhysicalOperator> phys_oper;
          local_rc = phys_gen.create(*logic_oper, phys_oper);
          if (local_rc != RC::SUCCESS) {
            g_outer_tuple = nullptr;
            delete sel_stmt;
            value.set_boolean(false);
            return local_rc;
          }

          if (sel_stmt->has_aggregation()) {
            auto *agg_oper = new AggregationPhysicalOperator();
            for (const auto &af : sel_stmt->agg_fields()) {
              agg_oper->add_aggregation(af.agg_type, af.table, af.field_meta, af.alias);
            }
            agg_oper->add_child(std::move(phys_oper));
            phys_oper.reset(agg_oper);
          }

          Session *session = Session::current_session();
          Trx *trx = session ? session->current_trx() : nullptr;
          local_rc = phys_oper->open(trx);
          if (local_rc != RC::SUCCESS) {
            g_outer_tuple = nullptr;
            delete sel_stmt;
            value.set_boolean(false);
            return local_rc;
          }

          std::vector<Value> result_values;
          int row_count = 0;
          while ((local_rc = phys_oper->next()) == RC::SUCCESS) {
            Tuple *tuple = phys_oper->current_tuple();
            if (tuple == nullptr) break;
            row_count++;
            Value v;
            local_rc = tuple->cell_at(0, v);
            if (local_rc == RC::SUCCESS) result_values.push_back(v);
          }
          phys_oper->close();
          delete sel_stmt;
          g_outer_tuple = nullptr;

          // Apply comparison logic (same as the non-correlated path)
          if (is_exists_op) {
            bool has_rows = (row_count > 0);
            if (comp == EXISTS_OP)
              value.set_boolean(has_rows);
            else
              value.set_boolean(!has_rows);
            return RC::SUCCESS;
          }

          if (is_in_op) {
            auto make_left = [&]() -> std::unique_ptr<Expression> {
              if (right_is_subquery) {
                if (other_obj.is_attr)
                  return std::unique_ptr<Expression>(new FieldExpr(other_obj.field));
                else
                  return std::unique_ptr<Expression>(new ValueExpr(other_obj.value));
              } else {
                if (other_obj.is_attr)
                  return std::unique_ptr<Expression>(new FieldExpr(other_obj.field));
                else
                  return std::unique_ptr<Expression>(new ValueExpr(other_obj.value));
              }
            };

            if (result_values.empty()) {
              value.set_boolean(comp == NOT_IN);
            } else if (comp == IN_OP) {
              bool found = false;
              for (auto &rv : result_values) {
                bool eq = false;
                Value left_val;
                if (right_is_subquery) {
                  if (other_obj.is_attr) {
                    FieldExpr fe(other_obj.field);
                    fe.get_value(outer_tuple, left_val);
                  } else {
                    left_val = other_obj.value;
                  }
                } else {
                  if (other_obj.is_attr) {
                    FieldExpr fe(other_obj.field);
                    fe.get_value(outer_tuple, left_val);
                  } else {
                    left_val = other_obj.value;
                  }
                }
                eq = (left_val.compare(rv) == 0);
                if (eq) { found = true; break; }
              }
              value.set_boolean(found);
            } else {
              bool all_neq = true;
              for (auto &rv : result_values) {
                Value left_val;
                if (right_is_subquery) {
                  if (other_obj.is_attr) {
                    FieldExpr fe(other_obj.field);
                    fe.get_value(outer_tuple, left_val);
                  } else {
                    left_val = other_obj.value;
                  }
                } else {
                  if (other_obj.is_attr) {
                    FieldExpr fe(other_obj.field);
                    fe.get_value(outer_tuple, left_val);
                  } else {
                    left_val = other_obj.value;
                  }
                }
                if (left_val.compare(rv) == 0) { all_neq = false; break; }
              }
              value.set_boolean(all_neq);
            }
            return RC::SUCCESS;
          }

          // Scalar subquery
          if (row_count > 1) { value.set_boolean(false); return RC::SUCCESS; }
          if (result_values.empty()) { value.set_boolean(false); return RC::SUCCESS; }

          Value scalar_val = result_values[0];
          Value other_val;
          if (right_is_subquery) {
            if (other_obj.is_attr) {
              FieldExpr fe(other_obj.field);
              fe.get_value(outer_tuple, other_val);
            } else {
              other_val = other_obj.value;
            }
          } else {
            if (other_obj.is_attr) {
              FieldExpr fe(other_obj.field);
              fe.get_value(outer_tuple, other_val);
            } else {
              other_val = other_obj.value;
            }
          }
          int cmp = other_val.compare(scalar_val);
          bool result = false;
          switch (comp) {
            case EQUAL_TO: result = (cmp == 0); break;
            case LESS_THAN: result = (cmp < 0); break;
            case GREAT_THAN: result = (cmp > 0); break;
            case LESS_EQUAL: result = (cmp <= 0); break;
            case GREAT_EQUAL: result = (cmp >= 0); break;
            case NOT_EQUAL: result = (cmp != 0); break;
            default: break;
          }
          value.set_boolean(result);
          return RC::SUCCESS;
        };

        filter_unit = new FilterUnit;
        filter_unit->set_custom_expr(new LambdaExpr(std::move(eval_func)));
        delete left_e; delete right_e;
        return rc;
      }

      auto *sel_stmt = static_cast<SelectStmt *>(inner_stmt);
      LOG_INFO("subquery: stmt created, tables=%d", (int)sel_stmt->tables().size());

      LogicalPlanGenerator logic_gen;
      std::unique_ptr<LogicalOperator> logic_oper;
      rc = logic_gen.create(sel_stmt, logic_oper);
      if (rc != RC::SUCCESS) {
        LOG_ERROR("subquery: failed to create subquery logical plan. rc=%s", strrc(rc));
        delete sel_stmt; delete left_e; delete right_e;
        return rc;
      }
      LOG_INFO("subquery: logical plan created");

      PhysicalPlanGenerator phys_gen;
      std::unique_ptr<PhysicalOperator> phys_oper;
      rc = phys_gen.create(*logic_oper, phys_oper);
      if (rc != RC::SUCCESS) {
        LOG_ERROR("subquery: failed to create subquery physical plan. rc=%s", strrc(rc));
        delete sel_stmt; delete left_e; delete right_e;
        return rc;
      }
      LOG_INFO("subquery: physical plan created");

      // 聚合查询包一层聚合算子（与 optimize_stage.cpp 一致）
      if (sel_stmt->has_aggregation()) {
        AggregationPhysicalOperator *agg_oper = new AggregationPhysicalOperator();
        for (const AggregationField &af : sel_stmt->agg_fields()) {
          agg_oper->add_aggregation(af.agg_type, af.table, af.field_meta, af.alias);
        }
        agg_oper->add_child(std::move(phys_oper));
        phys_oper.reset(agg_oper);
      }

      Session *session = Session::current_session();
      Trx *trx = session ? session->current_trx() : nullptr;
      LOG_INFO("subquery: session=%p trx=%p", session, trx);
      rc = phys_oper->open(trx);
      if (rc != RC::SUCCESS) {
        LOG_ERROR("subquery: failed to open subquery operator. rc=%s", strrc(rc));
        delete sel_stmt; delete left_e; delete right_e;
        return rc;
      }
      LOG_INFO("subquery: operator opened, collecting results");

      // 收集子查询结果的第一列
      std::vector<Value> result_values;
      int row_count = 0;
      while ((rc = phys_oper->next()) == RC::SUCCESS) {
        Tuple *tuple = phys_oper->current_tuple();
        if (tuple == nullptr) break;
        row_count++;
        Value v;
        rc = tuple->cell_at(0, v);
        if (rc == RC::SUCCESS) {
          result_values.push_back(v);
        }
      }
      phys_oper->close();
      delete sel_stmt;
      rc = RC::SUCCESS;  // 子查询执行成功，重置 rc（while 循环结束后 rc 是 EOF）

      if (is_exists_op) {
        // EXISTS / NOT EXISTS
        bool has_rows = (row_count > 0);
        if (comp == EXISTS_OP) {
          if (has_rows) {
            filter_unit = new FilterUnit;
            filter_unit->set_custom_expr(new ValueExpr(Value(true)));
          } else {
            filter_unit = new FilterUnit;
            filter_unit->set_custom_expr(new ValueExpr(Value(false)));
          }
        } else {
          if (!has_rows) {
            filter_unit = new FilterUnit;
            filter_unit->set_custom_expr(new ValueExpr(Value(true)));
          } else {
            filter_unit = new FilterUnit;
            filter_unit->set_custom_expr(new ValueExpr(Value(false)));
          }
        }
        delete left_e; delete right_e;
        return rc;
      }

      if (is_in_op) {
        // 检查子查询是否使用了 SELECT *（多列），IN/NOT IN 只允许单列
        const auto &sel_node = *sq->select_node();
        bool has_star = false;
        int col_count = 0;
        if (!sel_node.attributes.empty()) {
          for (auto &a : sel_node.attributes) {
            if (a.attribute_name == "*") { has_star = true; break; }
          }
          col_count = (int)sel_node.attributes.size();
        }
        if (!sel_node.expressions.empty()) {
          col_count = (int)sel_node.expressions.size();
        }
        if (has_star || col_count > 1) {
          delete left_e; delete right_e;
          return RC::INVALID_ARGUMENT;
        }

        // IN / NOT IN — rebuild the left-side expression from FilterObj
        auto make_left_expr = [&]() -> std::unique_ptr<Expression> {
          const FilterObj &obj = right_is_subquery ? left_obj : right_obj;
          if (obj.is_attr) {
            return std::unique_ptr<Expression>(new FieldExpr(obj.field));
          } else {
            return std::unique_ptr<Expression>(new ValueExpr(obj.value));
          }
        };
        if (result_values.empty()) {
          bool empty_is_true = (comp == NOT_IN);
          filter_unit = new FilterUnit;
          if (empty_is_true) {
            filter_unit->set_custom_expr(new ValueExpr(Value(true)));
          } else {
            filter_unit->set_custom_expr(new ValueExpr(Value(false)));
          }
        } else if (comp == IN_OP) {
          std::vector<std::unique_ptr<Expression>> or_children;
          for (auto &rv : result_values) {
            or_children.emplace_back(
              new ComparisonExpr(EQUAL_TO, make_left_expr(),
                std::unique_ptr<Expression>(new ValueExpr(rv))));
          }
          auto *or_expr = new ConjunctionExpr(ConjunctionExpr::Type::OR, or_children);
          filter_unit = new FilterUnit;
          filter_unit->set_custom_expr(or_expr);
        } else {
          std::vector<std::unique_ptr<Expression>> and_children;
          for (auto &rv : result_values) {
            and_children.emplace_back(
              new ComparisonExpr(NOT_EQUAL, make_left_expr(),
                std::unique_ptr<Expression>(new ValueExpr(rv))));
          }
          auto *and_expr = new ConjunctionExpr(ConjunctionExpr::Type::AND, and_children);
          filter_unit = new FilterUnit;
          filter_unit->set_custom_expr(and_expr);
        }
        delete left_e; delete right_e;
        return rc;
      }

      // 标量子查询（=, <, >, <=, >=, <>）
      if (row_count > 1) {
        LOG_WARN("scalar subquery returned more than one row");
        delete left_e; delete right_e;
        return RC::INVALID_ARGUMENT;
      }
      if (result_values.empty()) {
        // 空子查询结果 → 比较永远为 false
        filter_unit = new FilterUnit;
        filter_unit->set_custom_expr(new ValueExpr(Value(false)));
        delete left_e; delete right_e;
        return RC::SUCCESS;
      }

      // 替换 SubQueryExpr 为 ValueExpr
      Value scalar_val = result_values[0];
      Expression *val_expr = new ValueExpr(scalar_val);
      if (right_is_subquery) {
        delete right_e;
        right_e = val_expr;
        right_simple = true;
        right_obj.init_value(scalar_val);
      } else {
        delete left_e;
        left_e = val_expr;
        left_simple = true;
        left_obj.init_value(scalar_val);
      }

      // Handle the case where BOTH sides are subqueries
      bool other_is_subquery = (left_is_subquery && right_is_subquery);
      if (other_is_subquery) {
        Expression *other_e = right_is_subquery ? left_e : right_e;
        bool &other_simple = right_is_subquery ? left_simple : right_simple;
        FilterObj &other_obj = right_is_subquery ? left_obj : right_obj;
        auto *other_sq = static_cast<SubQueryExpr *>(other_e);
        Stmt *other_stmt = nullptr;
        RC other_rc = SelectStmt::create(db, *other_sq->select_node(), other_stmt, outer_table_map);
        if (other_rc != RC::SUCCESS && outer_table_map == nullptr) {
          other_rc = SelectStmt::create(db, *other_sq->select_node(), other_stmt);
        }
        if (other_rc == RC::SUCCESS) {
          auto *other_sel = static_cast<SelectStmt *>(other_stmt);
          LogicalPlanGenerator other_lg;
          std::unique_ptr<LogicalOperator> other_lo;
          other_rc = other_lg.create(other_sel, other_lo);
          if (other_rc == RC::SUCCESS) {
            PhysicalPlanGenerator other_pg;
            std::unique_ptr<PhysicalOperator> other_po;
            other_rc = other_pg.create(*other_lo, other_po);
            if (other_rc == RC::SUCCESS) {
              if (other_sel->has_aggregation()) {
                auto *agg = new AggregationPhysicalOperator();
                for (const auto &af : other_sel->agg_fields()) {
                  agg->add_aggregation(af.agg_type, af.table, af.field_meta, af.alias);
                }
                agg->add_child(std::move(other_po));
                other_po.reset(agg);
              }
              Session *sess = Session::current_session();
              Trx *t = sess ? sess->current_trx() : nullptr;
              other_rc = other_po->open(t);
              if (other_rc == RC::SUCCESS) {
                std::vector<Value> other_vals;
                int other_rows = 0;
                while (other_po->next() == RC::SUCCESS) {
                  Tuple *tp = other_po->current_tuple();
                  if (tp == nullptr) break;
                  other_rows++;
                  Value v;
                  if (tp->cell_at(0, v) == RC::SUCCESS) other_vals.push_back(v);
                }
                other_po->close();
                if (other_rows == 1 && !other_vals.empty()) {
                  delete other_e;
                  if (right_is_subquery) {
                    left_e = new ValueExpr(other_vals[0]);
                    left_simple = true;
                    left_obj.init_value(other_vals[0]);
                  } else {
                    right_e = new ValueExpr(other_vals[0]);
                    right_simple = true;
                    right_obj.init_value(other_vals[0]);
                  }
                }
              }
            }
          }
          delete other_sel;
        }
      }

      if (left_simple && right_simple) {
        filter_unit = new FilterUnit();
        filter_unit->set_comp(comp);
        filter_unit->set_left(left_obj);
        filter_unit->set_right(right_obj);
        return rc;
      }
    }

    if (left_simple && right_simple && !left_obj.is_attr && !right_obj.is_attr) {
      // 两边都是常量: 使用表达式路径以保留常量比较语义
      filter_unit = new FilterUnit();
      filter_unit->set_comp(comp);
      filter_unit->set_left_expr(new ValueExpr(left_obj.value));
      filter_unit->set_right_expr(new ValueExpr(right_obj.value));
    } else if (left_simple && right_simple) {
      // 两边都是简单类型且至少一边是字段: 使用 legacy 路径
      filter_unit = new FilterUnit();
      filter_unit->set_comp(comp);
      filter_unit->set_left(left_obj);
      filter_unit->set_right(right_obj);
    } else {
      // 复杂表达式: 使用表达式路径
      filter_unit = new FilterUnit();
      filter_unit->set_comp(comp);
      if (!left_simple && left_e) {
        filter_unit->set_left_expr(left_e);
      } else if (left_simple) {
        if (left_obj.is_attr) {
          filter_unit->set_left_expr(new FieldExpr(left_obj.field));
        } else {
          filter_unit->set_left_expr(new ValueExpr(left_obj.value));
        }
      }
      if (!right_simple && right_e) {
        filter_unit->set_right_expr(right_e);
      } else if (right_simple) {
        if (right_obj.is_attr) {
          filter_unit->set_right_expr(new FieldExpr(right_obj.field));
        } else {
          filter_unit->set_right_expr(new ValueExpr(right_obj.value));
        }
      }
    }
  }

  // Type validation (legacy path only)
  if (!filter_unit->is_expr_based() && !filter_unit->has_custom_expr()) {
    if (filter_unit->left().is_attr && !filter_unit->right().is_attr) {
      const FieldMeta *field = filter_unit->left().field.meta();
      if (field->type() == DATES && filter_unit->right().value.attr_type() == CHARS) {
        delete filter_unit; return RC::INVALID_ARGUMENT;
      }
    } else if (!filter_unit->left().is_attr && filter_unit->right().is_attr) {
      const FieldMeta *field = filter_unit->right().field.meta();
      if (field->type() == DATES && filter_unit->left().value.attr_type() == CHARS) {
        delete filter_unit; return RC::INVALID_ARGUMENT;
      }
    }
  }

  return rc;
}

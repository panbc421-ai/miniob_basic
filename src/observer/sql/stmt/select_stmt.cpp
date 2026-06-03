#include "sql/stmt/select_stmt.h"
#include "sql/stmt/filter_stmt.h"
#include "common/log/log.h"
#include "common/lang/string.h"
#include "storage/db/db.h"
#include "storage/table/table.h"
#include "sql/expr/expression.h"

// Walk expression tree and collect AggregationExpr nodes, replacing them
// with FieldExpr references for the aggregation output tuple.
static void collect_aggregations(Expression *&expr, Table *default_table,
    std::unordered_map<std::string, Table *> *table_map,
    std::vector<AggregationField> &agg_fields, bool &has_agg)
{
  if (expr == nullptr) return;
  if (expr->type() == ExprType::AGGREGATION) {
    auto *agg = static_cast<AggregationExpr *>(expr);
    has_agg = true;
    const std::string &tn = agg->table_name();
    const std::string &fn = agg->field_name();
    if (fn == "*") {
      AggregationField af;
      af.agg_type = AGG_COUNT;
      af.field_meta = nullptr;
      af.table = nullptr;
      af.alias = "count(*)";
      agg_fields.push_back(af);
      // Replace with a dummy field that will be resolved by index
      return;
    }
    Table *table = default_table;
    if (!tn.empty() && table_map != nullptr) {
      auto it = table_map->find(tn);
      if (it != table_map->end()) table = it->second;
    }
    if (table == nullptr) return;
    const FieldMeta *fm = table->table_meta().field(fn.c_str());
    if (fm == nullptr) return;
    std::string alias = fn;  // use original field name as alias for lookup
    AggregationField af;
    af.agg_type = agg->agg_type();
    af.table = table;
    af.field_meta = fm;
    af.alias = alias;
    agg_fields.push_back(af);
    // Replace AggregationExpr with FieldExpr
    auto *fe = new FieldExpr(table, fm);
    fe->set_name(expr->name());
    delete expr;
    expr = fe;
    return;
  }
  if (expr->type() == ExprType::ARITHMETIC) {
    auto *ae = static_cast<ArithmeticExpr *>(expr);
    auto &left = const_cast<std::unique_ptr<Expression> &>(ae->left());
    auto &right = const_cast<std::unique_ptr<Expression> &>(ae->right());
    Expression *lptr = left.get();
    Expression *rptr = right.get();
    collect_aggregations(lptr, default_table, table_map, agg_fields, has_agg);
    collect_aggregations(rptr, default_table, table_map, agg_fields, has_agg);
    return;
  }
  if (expr->type() == ExprType::CAST) {
    auto *ce = static_cast<CastExpr *>(expr);
    Expression *child = ce->child().get();
    collect_aggregations(child, default_table, table_map, agg_fields, has_agg);
    return;
  }
  if (expr->type() == ExprType::FUNCTION) {
    auto *fn = static_cast<FunctionExpr *>(expr);
    for (auto &arg : fn->args()) {
      Expression *a = arg.get();
      collect_aggregations(a, default_table, table_map, agg_fields, has_agg);
    }
    return;
  }
}

SelectStmt::~SelectStmt()
{
  if (nullptr != filter_stmt_) {
    delete filter_stmt_;
    filter_stmt_ = nullptr;
  }
  for (auto &se : select_exprs_) {
    delete se.expr;
    se.expr = nullptr;
  }
}

static void wildcard_fields(Table *table, std::vector<Field> &field_metas)
{
  const TableMeta &table_meta = table->table_meta();
  const int field_num = table_meta.field_num();
  for (int i = table_meta.sys_field_num(); i < field_num; i++) {
    field_metas.push_back(Field(table, table_meta.field(i)));
  }
}

RC SelectStmt::create(Db *db, const SelectSqlNode &select_sql, Stmt *&stmt,
    std::unordered_map<std::string, Table *> *outer_table_map)
{
  if (nullptr == db) {
    LOG_WARN("invalid argument. db is null");
    return RC::INVALID_ARGUMENT;
  }

  std::vector<Table *> tables;
  std::unordered_map<std::string, Table *> table_map;
  for (size_t i = 0; i < select_sql.relations.size(); i++) {
    const char *table_name = select_sql.relations[i].c_str();
    if (nullptr == table_name) {
      LOG_WARN("invalid argument. relation name is null. index=%d", i);
      return RC::INVALID_ARGUMENT;
    }
    Table *table = db->find_table(table_name);
    if (nullptr == table) {
      LOG_WARN("no such table. db=%s, table_name=%s", db->name(), table_name);
      return RC::SCHEMA_TABLE_NOT_EXIST;
    }
    tables.push_back(table);
    table_map.insert(std::pair<std::string, Table *>(table_name, table));
  }

  // Add alias entries to table_map (alias -> table)
  for (size_t i = 0; i < select_sql.aliases.size() && i < select_sql.relations.size(); i++) {
    const std::string &alias = select_sql.aliases[i];
    if (!alias.empty()) {
      const std::string &table_name = select_sql.relations[i];
      auto it = table_map.find(table_name);
      if (it != table_map.end()) {
        table_map.insert(std::pair<std::string, Table *>(alias, it->second));
      }
    }
  }

  Table *default_table = nullptr;
  if (tables.size() == 1) {
    default_table = tables[0];
  }

  std::vector<Field> query_fields;
  bool has_aggregation = false;
  std::vector<AggregationField> agg_fields;

  // === 处理 select_sql.attributes（legacy 路径） ===
  for (int i = static_cast<int>(select_sql.attributes.size()) - 1; i >= 0; i--) {
    const RelAttrSqlNode &relation_attr = select_sql.attributes[i];

    if (relation_attr.aggregation_type != AGG_NONE) {
      has_aggregation = true;
      AggregationField agg;
      agg.agg_type = relation_attr.aggregation_type;
      if (relation_attr.attribute_name == "*") {
        agg.field_meta = nullptr;
        agg.table = nullptr;
        agg.alias = "count(*)";
      } else {
        Table *table = tables[0];
        if (!common::is_blank(relation_attr.relation_name.c_str())) {
          auto iter = table_map.find(relation_attr.relation_name);
          if (iter == table_map.end()) {
            LOG_WARN("no such table: %s", relation_attr.relation_name.c_str());
            return RC::SCHEMA_TABLE_NOT_EXIST;
          }
          table = iter->second;
        }
        const FieldMeta *field_meta = table->table_meta().field(relation_attr.attribute_name.c_str());
        if (field_meta == nullptr) {
          LOG_WARN("no such field: %s", relation_attr.attribute_name.c_str());
          return RC::SCHEMA_FIELD_NOT_EXIST;
        }
        agg.field_meta = field_meta;
        agg.table = table;
        std::string func_name;
        switch (relation_attr.aggregation_type) {
          case AGG_MAX: func_name = "max"; break;
          case AGG_MIN: func_name = "min"; break;
          case AGG_COUNT: func_name = "count"; break;
          case AGG_AVG: func_name = "avg"; break;
          case AGG_SUM: func_name = "sum"; break;
          default: func_name = ""; break;
        }
        agg.alias = func_name + "(" + relation_attr.attribute_name + ")";
      }
      agg_fields.push_back(agg);
      continue;
    }

    if (has_aggregation) {
      LOG_WARN("cannot mix aggregation and non-aggregation fields");
      return RC::INVALID_ARGUMENT;
    }

    if (common::is_blank(relation_attr.relation_name.c_str()) &&
        0 == strcmp(relation_attr.attribute_name.c_str(), "*")) {
      for (Table *table : tables) {
        wildcard_fields(table, query_fields);
      }
    } else if (!common::is_blank(relation_attr.relation_name.c_str())) {
      const char *table_name = relation_attr.relation_name.c_str();
      const char *field_name = relation_attr.attribute_name.c_str();
      if (0 == strcmp(table_name, "*")) {
        if (0 != strcmp(field_name, "*")) return RC::SCHEMA_FIELD_MISSING;
        for (Table *table : tables) wildcard_fields(table, query_fields);
      } else {
        auto iter = table_map.find(table_name);
        if (iter == table_map.end()) return RC::SCHEMA_FIELD_MISSING;
        Table *table = iter->second;
        if (0 == strcmp(field_name, "*")) {
          wildcard_fields(table, query_fields);
        } else {
          const FieldMeta *field_meta = table->table_meta().field(field_name);
          if (nullptr == field_meta) return RC::SCHEMA_FIELD_MISSING;
          query_fields.push_back(Field(table, field_meta));
        }
      }
    } else {
      if (tables.size() != 1) {
        LOG_WARN("invalid. I do not know the attr's table. attr=%s", relation_attr.attribute_name.c_str());
        return RC::SCHEMA_FIELD_MISSING;
      }
      Table *table = tables[0];
      const FieldMeta *field_meta = table->table_meta().field(relation_attr.attribute_name.c_str());
      if (nullptr == field_meta) {
        LOG_WARN("no such field. field=%s.%s.%s", db->name(), table->name(), relation_attr.attribute_name.c_str());
        return RC::SCHEMA_FIELD_MISSING;
      }
      query_fields.push_back(Field(table, field_meta));
    }
  }

  // === 处理 select_sql.expressions ===
  std::vector<SelectExprNode> select_exprs;
  if (!select_sql.expressions.empty()) {
    for (int i = static_cast<int>(select_sql.expressions.size()) - 1; i >= 0; i--) {
      const auto &sel_expr = select_sql.expressions[i];

      // 处理聚合
      if (sel_expr.agg_type != AGG_NONE) {
        has_aggregation = true;
        AggregationField agg;
        agg.agg_type = sel_expr.agg_type;

        if (sel_expr.agg_field == "*") {
          agg.field_meta = nullptr;
          agg.table = nullptr;
          agg.alias = "count(*)";
        } else {
          if (tables.empty()) return RC::INVALID_ARGUMENT;
          Table *table = tables[0];
          if (!common::is_blank(sel_expr.agg_table.c_str())) {
            auto iter = table_map.find(sel_expr.agg_table);
            if (iter == table_map.end()) return RC::SCHEMA_TABLE_NOT_EXIST;
            table = iter->second;
          }
          const FieldMeta *field_meta = table->table_meta().field(sel_expr.agg_field.c_str());
          if (field_meta == nullptr) return RC::SCHEMA_FIELD_MISSING;
          agg.field_meta = field_meta;
          agg.table = table;
          std::string func_name;
          switch (sel_expr.agg_type) {
            case AGG_MAX: func_name = "max"; break;
            case AGG_MIN: func_name = "min"; break;
            case AGG_COUNT: func_name = "count"; break;
            case AGG_AVG: func_name = "avg"; break;
            case AGG_SUM: func_name = "sum"; break;
            default: func_name = ""; break;
          }
          agg.alias = func_name + "(" + sel_expr.agg_field + ")";
        }
        agg_fields.push_back(agg);
        continue;
      }

      // Extract aggregation functions from expression tree before resolving
      if (sel_expr.expr != nullptr) {
        Expression *eptr = sel_expr.expr;
        collect_aggregations(eptr, default_table, &table_map, agg_fields, has_aggregation);
        const_cast<SelectSqlNode &>(select_sql).expressions[i].expr = eptr;
      }

      std::unique_ptr<Expression> e(sel_expr.expr);
      const_cast<SelectSqlNode &>(select_sql).expressions[i].expr = nullptr;
      std::string alias = sel_expr.alias;

      RC rc = resolve_expression(e, default_table, &table_map);
      if (rc != RC::SUCCESS) {
        LOG_WARN("failed to resolve expression in select list");
        return rc;
      }

      if (e->type() == ExprType::FIELD) {
        auto *fe = static_cast<FieldExpr *>(e.get());
        query_fields.push_back(fe->field());
      } else {
        SelectExprNode resolved;
        resolved.alias = alias;
        resolved.expr = e.release();
        select_exprs.push_back(std::move(resolved));
      }
    }
    const_cast<SelectSqlNode &>(select_sql).expressions.clear();
  }

  if (has_aggregation && !query_fields.empty() && select_sql.group_by.empty()) {
    LOG_WARN("cannot mix aggregation and non-aggregation fields without GROUP BY");
    return RC::INVALID_ARGUMENT;
  }

  // Resolve group_by fields from parsed GROUP BY clause
  std::vector<Field> group_by_fields;
  for (const RelAttrSqlNode &gb : select_sql.group_by) {
    Table *table = default_table;
    if (!common::is_blank(gb.relation_name.c_str())) {
      auto iter = table_map.find(gb.relation_name);
      if (iter == table_map.end()) {
        LOG_WARN("no such table for GROUP BY: %s", gb.relation_name.c_str());
        return RC::SCHEMA_TABLE_NOT_EXIST;
      }
      table = iter->second;
    }
    if (table == nullptr) {
      return RC::SCHEMA_FIELD_MISSING;
    }
    const FieldMeta *field_meta = table->table_meta().field(gb.attribute_name.c_str());
    if (nullptr == field_meta) {
      LOG_WARN("no such field for GROUP BY: %s.%s", table->name(), gb.attribute_name.c_str());
      return RC::SCHEMA_FIELD_MISSING;
    }
    group_by_fields.push_back(Field(table, field_meta));
  }

  FilterStmt *filter_stmt = nullptr;
  RC rc = FilterStmt::create(db,
      default_table,
      &table_map,
      select_sql.conditions.data(),
      static_cast<int>(select_sql.conditions.size()),
      filter_stmt,
      outer_table_map);
  if (rc != RC::SUCCESS) {
    LOG_WARN("cannot construct filter stmt");
    return rc;
  }

  SelectStmt *select_stmt = new SelectStmt();
  select_stmt->tables_.swap(tables);
  select_stmt->query_fields_.swap(query_fields);
  select_stmt->filter_stmt_ = filter_stmt;
  select_stmt->agg_fields_.swap(agg_fields);
  select_stmt->has_aggregation_ = has_aggregation;
  select_stmt->order_by_ = select_sql.order_by;
  select_stmt->select_exprs_.swap(select_exprs);
  select_stmt->group_by_fields_.swap(group_by_fields);
  stmt = select_stmt;
  return RC::SUCCESS;
}

#include "sql/stmt/select_stmt.h"
#include "sql/stmt/filter_stmt.h"
#include "common/log/log.h"
#include "common/lang/string.h"
#include "storage/db/db.h"
#include "storage/table/table.h"

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

RC SelectStmt::create(Db *db, const SelectSqlNode &select_sql, Stmt *&stmt)
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

  std::vector<Field> query_fields;
  bool has_aggregation = false;
  std::vector<AggregationField> agg_fields;

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
        if (tables.empty()) {
          LOG_WARN("no table for aggregation");
          return RC::INVALID_ARGUMENT;
        }
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

    // 混合聚合和普通字段
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
        if (0 != strcmp(field_name, "*")) {
          LOG_WARN("invalid field name while table is *. attr=%s", field_name);
          return RC::SCHEMA_FIELD_MISSING;
        }
        for (Table *table : tables) {
          wildcard_fields(table, query_fields);
        }
      } else {
        auto iter = table_map.find(table_name);
        if (iter == table_map.end()) {
          LOG_WARN("no such table in from list: %s", table_name);
          return RC::SCHEMA_FIELD_MISSING;
        }
        Table *table = iter->second;
        if (0 == strcmp(field_name, "*")) {
          wildcard_fields(table, query_fields);
        } else {
          const FieldMeta *field_meta = table->table_meta().field(field_name);
          if (nullptr == field_meta) {
            LOG_WARN("no such field. field=%s.%s.%s", db->name(), table->name(), field_name);
            return RC::SCHEMA_FIELD_MISSING;
          }
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

  // 检查是否有普通字段混在聚合里
  if (has_aggregation && !query_fields.empty()) {
    LOG_WARN("cannot mix aggregation and non-aggregation fields");
    return RC::INVALID_ARGUMENT;
  }

  LOG_INFO("got %d tables in from stmt and %d fields in query stmt", tables.size(), query_fields.size());

  Table *default_table = nullptr;
  if (tables.size() == 1) {
    default_table = tables[0];
  }

  FilterStmt *filter_stmt = nullptr;
  RC rc = FilterStmt::create(db,
      default_table,
      &table_map,
      select_sql.conditions.data(),
      static_cast<int>(select_sql.conditions.size()),
      filter_stmt);
  if (rc != RC::SUCCESS) {
    LOG_WARN("cannot construct filter stmt");
    return rc;
  }

  // 处理表达式 select items
  std::vector<SelectExprNode> select_exprs;
  if (!select_sql.expressions.empty()) {
    for (size_t i = 0; i < select_sql.expressions.size(); i++) {
      SelectExprNode resolved;
      resolved.alias = select_sql.expressions[i].alias;
      std::unique_ptr<Expression> e(select_sql.expressions[i].expr);
      const_cast<SelectSqlNode&>(select_sql).expressions[i].expr = nullptr;
      RC rc = resolve_expression(e, default_table, &table_map);
      if (rc != RC::SUCCESS) {
        LOG_WARN("failed to resolve expression in select list");
        return rc;
      }
      resolved.expr = e.release();
      select_exprs.push_back(std::move(resolved));
    }
    const_cast<SelectSqlNode&>(select_sql).expressions.clear();
  }

  SelectStmt *select_stmt = new SelectStmt();
  select_stmt->tables_.swap(tables);
  select_stmt->query_fields_.swap(query_fields);
  select_stmt->filter_stmt_ = filter_stmt;
  select_stmt->agg_fields_.swap(agg_fields);
  select_stmt->has_aggregation_ = has_aggregation;
  select_stmt->order_by_ = select_sql.order_by;
  select_stmt->select_exprs_.swap(select_exprs);
  stmt = select_stmt;
  return RC::SUCCESS;
}
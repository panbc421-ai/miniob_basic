#include "sql/operator/sort_physical_operator.h"
#include "common/log/log.h"
#include <algorithm>

RC SortPhysicalOperator::open(Trx *trx)
{
  if (children_.empty()) return RC::SUCCESS;
  return children_[0]->open(trx);
}

RC SortPhysicalOperator::next()
{
  if (!sorted_) {
    sorted_ = true;

    if (children_.empty()) return RC::RECORD_EOF;
    PhysicalOperator *child = children_[0].get();

    // Single pass: collect all tuples with their sort keys
    RC rc = RC::SUCCESS;
    while ((rc = child->next()) == RC::SUCCESS) {
      Tuple *tuple = child->current_tuple();
      SortEntry entry;
      int n = tuple->cell_num();

      // Extract sort key values for each ORDER BY field
      for (size_t i = 0; i < order_by_.size(); i++) {
        const OrderByNode &ob = order_by_[i];
        Value val;
        std::string alias = ob.relation_name.empty()
            ? ob.attribute_name
            : ob.relation_name + "." + ob.attribute_name;
        TupleCellSpec spec(ob.relation_name.c_str(),
                           ob.attribute_name.c_str(),
                           alias.c_str());
        RC rc2 = tuple->find_cell(spec, val);
        if (rc2 != RC::SUCCESS) {
          // Fallback: try matching without table prefix
          TupleCellSpec spec2("", ob.attribute_name.c_str(),
                              ob.attribute_name.c_str());
          tuple->find_cell(spec2, val);
        }
        entry.sort_keys.push_back(val);
      }

      // Copy all cell values in order
      for (int i = 0; i < n; i++) {
        Value val;
        tuple->cell_at(i, val);
        entry.cells.push_back(val);
      }

      entries_.push_back(entry);
    }

    if (entries_.empty()) {
      return RC::RECORD_EOF;
    }

    // Sort entries based on order_by_ specs
    std::stable_sort(entries_.begin(), entries_.end(),
        [this](const SortEntry &a, const SortEntry &b) {
          for (size_t i = 0; i < order_by_.size(); i++) {
            const Value &va = a.sort_keys[i];
            const Value &vb = b.sort_keys[i];

            int cmp = 0;
            AttrType ta = va.attr_type();
            AttrType tb = vb.attr_type();

            if (ta == INTS && tb == INTS) {
              int ia = va.get_int(), ib = vb.get_int();
              cmp = (ia < ib) ? -1 : (ia > ib) ? 1 : 0;
            } else if (ta == FLOATS && tb == FLOATS) {
              float fa = va.get_float(), fb = vb.get_float();
              cmp = (fa < fb) ? -1 : (fa > fb) ? 1 : 0;
            } else if (ta == CHARS && tb == CHARS) {
              cmp = std::string(va.get_string()).compare(vb.get_string());
            } else if (ta == DATES && tb == DATES) {
              int da = va.get_date(), db = vb.get_date();
              cmp = (da < db) ? -1 : (da > db) ? 1 : 0;
            } else if (ta == INTS && tb == FLOATS) {
              float fa = (float)va.get_int(), fb = vb.get_float();
              cmp = (fa < fb) ? -1 : (fa > fb) ? 1 : 0;
            } else if (ta == FLOATS && tb == INTS) {
              float fa = va.get_float(), fb = (float)vb.get_int();
              cmp = (fa < fb) ? -1 : (fa > fb) ? 1 : 0;
            }

            if (cmp != 0) {
              return order_by_[i].is_asc ? (cmp < 0) : (cmp > 0);
            }
          }
          return false;
        });

    current_index_ = 0;
    result_tuple_.set_cells(entries_[0].cells);
    return RC::SUCCESS;
  }

  current_index_++;
  if (current_index_ >= (int)entries_.size()) {
    return RC::RECORD_EOF;
  }

  result_tuple_.set_cells(entries_[current_index_].cells);
  return RC::SUCCESS;
}

RC SortPhysicalOperator::close()
{
  if (!children_.empty()) children_[0]->close();
  return RC::SUCCESS;
}

Tuple *SortPhysicalOperator::current_tuple()
{
  return &result_tuple_;
}

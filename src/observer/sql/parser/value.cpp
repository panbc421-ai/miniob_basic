/* Copyright (c) 2021 OceanBase and/or its affiliates. All rights reserved.
miniob is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
         http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

//
// Created by WangYunlai on 2023/06/28.
//

#include <sstream>
#include <cmath>
#include "sql/parser/value.h"
#include "storage/field/field.h"
#include "common/log/log.h"
#include "common/lang/comparator.h"
#include "common/lang/string.h"
#include <cstring>

const char *ATTR_TYPE_NAME[] = {"undefined", "chars", "ints", "floats", "dates", "booleans", "texts", "nulls"};

const char *attr_type_to_string(AttrType type)
{
  if (type >= UNDEFINED && type <= NULLS) {
    return ATTR_TYPE_NAME[type];
  }
  return "unknown";
}
AttrType attr_type_from_string(const char *s)
{
  for (unsigned int i = 0; i < sizeof(ATTR_TYPE_NAME) / sizeof(ATTR_TYPE_NAME[0]); i++) {
    if (0 == strcmp(ATTR_TYPE_NAME[i], s)) {
      return (AttrType)i;
    }
  }
  return UNDEFINED;
}

Value::Value(int val)
{
  set_int(val);
}

Value::Value(float val)
{
  set_float(val);
  plain_float_format_ = true;
}

Value::Value(bool val)
{
  set_boolean(val);
}

Value::Value(const char *s, int len /*= 0*/)
{
  set_string(s, len);
}

void Value::set_data(char *data, int length)
{
  switch (attr_type_) {
    case CHARS:
    case TEXTS: {
      AttrType original_type = attr_type_;
      set_string(data, length);
      attr_type_ = original_type;
    } break;
    case INTS: {
      num_value_.int_value_ = *(int *)data;
      has_double_value_ = false;
      plain_float_format_ = false;
      length_ = length;
    } break;
    case FLOATS: {
      num_value_.float_value_ = *(float *)data;
      has_double_value_ = false;
      plain_float_format_ = true;
      length_ = length;
    } break;
    case BOOLEANS: {
      num_value_.bool_value_ = *(int *)data != 0;
      has_double_value_ = false;
      plain_float_format_ = false;
      length_ = length;
    } break;
    case DATES: {
  num_value_.date_value_ = *(int *)data;
  has_double_value_ = false;
  plain_float_format_ = false;
  length_ = length;
} break;
    default: {
      LOG_WARN("unknown data type: %d", attr_type_);
    } break;
  }
}
static bool check_date(int y, int m, int d)
{
  // Validate year/month/day ranges. Accept years in [1, 9999].
  if (y < 1 || y > 9999) {
    return false;
  }
  if (m < 1 || m > 12 || d < 1) {
    return false;
  }

  static const int days_of_month[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  bool leap = (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0);
  int max_day = days_of_month[m];
  if (m == 2 && leap) {
    max_day = 29;
  }
  return d <= max_day;
}
void Value::set_int(int val)
{
  attr_type_ = INTS;
  has_double_value_ = false;
  plain_float_format_ = false;
  num_value_.int_value_ = val;
  length_ = sizeof(val);
}

void Value::set_float(float val)
{
  attr_type_ = FLOATS;
  has_double_value_ = false;
  plain_float_format_ = false;
  num_value_.float_value_ = val;
  length_ = sizeof(val);
}
void Value::set_float(double val, bool plain_format)
{
  attr_type_ = FLOATS;
  num_value_.float_value_ = static_cast<float>(val);
  double_value_ = val;
  has_double_value_ = true;
  plain_float_format_ = plain_format;
  length_ = sizeof(num_value_.float_value_);
}
void Value::set_date(int val)
{
  attr_type_ = DATES;
  has_double_value_ = false;
  plain_float_format_ = false;
  num_value_.date_value_ = val;
  length_ = sizeof(val);
}

int Value::get_date() const
{
  return num_value_.date_value_;
}
void Value::set_boolean(bool val)
{
  attr_type_ = BOOLEANS;
  has_double_value_ = false;
  plain_float_format_ = false;
  num_value_.bool_value_ = val;
  length_ = sizeof(val);
}
void Value::set_string(const char *s, int len /*= 0*/)
{
  attr_type_ = CHARS;
  has_double_value_ = false;
  plain_float_format_ = false;
  if (len > 0) {
    len = strnlen(s, len);
    str_value_.assign(s, len);
  } else {
    str_value_.assign(s);
  }
  length_ = str_value_.length();
}

void Value::set_value(const Value &value)
{
  if (value.is_null()) {
    set_null(true);
    return;
  }
  switch (value.attr_type_) {
    case INTS: {
      set_int(value.get_int());
    } break;
    case FLOATS: {
      set_float(value.get_double(), value.plain_float_format());
    } break;
    case CHARS:
    case TEXTS: {
      set_string(value.get_string().c_str());
    } break;
    case DATES: {
      set_date(value.get_date());
    } break;
    case BOOLEANS: {
      set_boolean(value.get_boolean());
    } break;
    case UNDEFINED: {
      ASSERT(false, "got an invalid value type");
    } break;

  }
}

const char *Value::data() const
{
  switch (attr_type_) {
    case CHARS:
    case TEXTS: {
      return str_value_.c_str();
    } break;
    default: {
      return (const char *)&num_value_;
    } break;
  }
}

static std::string plain_double_to_str(double v)
{
  char buf[256];
  snprintf(buf, sizeof(buf), "%.2f", v);
  size_t len = strlen(buf);
  while (len > 0 && buf[len - 1] == '0') {
    len--;
  }
  if (len > 0 && buf[len - 1] == '.') {
    len--;
  }

  return std::string(buf, len);
}

std::string Value::to_string() const
{
  if (is_null_) {
    return "NULL";
  }
  std::stringstream os;
  switch (attr_type_) {
    case INTS: {
      os << num_value_.int_value_;
    } break;
    case FLOATS: {
      double v = has_double_value_ ? double_value_ : num_value_.float_value_;
      os << (plain_float_format_ ? plain_double_to_str(v) : common::double_to_str(v));
    } break;
    case BOOLEANS: {
      os << num_value_.bool_value_;
    } break;
    case CHARS:
    case TEXTS: {
      os << str_value_;
    } break;
    case DATES: {
  char buf[16];
   int y = num_value_.date_value_ / 10000;
  int m = (num_value_.date_value_ % 10000) / 100;
  int d = num_value_.date_value_ % 100;
  snprintf(buf, sizeof(buf), "%04d-%02d-%02d", y, m, d);
  os << buf;
    } break;
    default: {
      LOG_WARN("unsupported attr type: %d", attr_type_);
    } break;
  }
  return os.str();
}

int Value::compare(const Value &other) const
{
  // NULL comparisons always return -1 (not equal in SQL semantics)
  if (this->is_null_ || other.is_null_) {
    return -2; // special value for "unknown"
  }
  if (this->attr_type_ == other.attr_type_) {
    switch (this->attr_type_) {
      case INTS: {
        return common::compare_int((void *)&this->num_value_.int_value_, (void *)&other.num_value_.int_value_);
      } break;
      case FLOATS: {
        return common::compare_float((void *)&this->num_value_.float_value_, (void *)&other.num_value_.float_value_);
      } break;
      case CHARS:
      case TEXTS: {
        return common::compare_string((void *)this->str_value_.c_str(),
            this->str_value_.length(),
            (void *)other.str_value_.c_str(),
            other.str_value_.length());
      } break;
      case DATES: {
        return common::compare_int((void *)&this->num_value_.date_value_, (void *)&other.num_value_.date_value_);
      } break;
      case BOOLEANS: {
        return common::compare_int((void *)&this->num_value_.bool_value_, (void *)&other.num_value_.bool_value_);
      }
      default: {
        LOG_WARN("unsupported type: %d", this->attr_type_);
      }
    }
  } else if (this->attr_type_ == INTS && other.attr_type_ == FLOATS) {
    float this_data = this->num_value_.int_value_;
    return common::compare_float((void *)&this_data, (void *)&other.num_value_.float_value_);
  } else if (this->attr_type_ == FLOATS && other.attr_type_ == INTS) {
    float other_data = other.num_value_.int_value_;
    return common::compare_float((void *)&this->num_value_.float_value_, (void *)&other_data);
  } else if ((this->attr_type_ == CHARS || this->attr_type_ == TEXTS) && other.attr_type_ == FLOATS) {
    try {
      float this_data = (float)std::stod(this->str_value_);
      return common::compare_float((void *)&this_data, (void *)&other.num_value_.float_value_);
    } catch (...) {
      return -1;
    }
  } else if (this->attr_type_ == FLOATS && (other.attr_type_ == CHARS || other.attr_type_ == TEXTS)) {
    try {
      float other_data = (float)std::stod(other.str_value_);
      return common::compare_float((void *)&this->num_value_.float_value_, (void *)&other_data);
    } catch (...) {
      return -1;
    }
  } else if ((this->attr_type_ == CHARS || this->attr_type_ == TEXTS) && other.attr_type_ == INTS) {
    try {
      float this_data = (float)std::stod(this->str_value_);
      float other_data = (float)other.num_value_.int_value_;
      return common::compare_float((void *)&this_data, (void *)&other_data);
    } catch (...) {
      return -1;
    }
  } else if (this->attr_type_ == INTS && (other.attr_type_ == CHARS || other.attr_type_ == TEXTS)) {
    try {
      float this_data = (float)this->num_value_.int_value_;
      float other_data = (float)std::stod(other.str_value_);
      return common::compare_float((void *)&this_data, (void *)&other_data);
    } catch (...) {
      return -1;
    }
  }
  LOG_WARN("not supported");
  return -1;  // TODO return rc?
}

int Value::get_int() const
{
  switch (attr_type_) {
    case CHARS:
    case TEXTS: {
      try {
        return (int)(std::stol(str_value_));
      } catch (std::exception const &ex) {
        LOG_TRACE("failed to convert string to number. s=%s, ex=%s", str_value_.c_str(), ex.what());
        return 0;
      }
    }
    case INTS: {
      return num_value_.int_value_;
    }
    case FLOATS: {
      return (int)(num_value_.float_value_);
    }
    case BOOLEANS: {
      return (int)(num_value_.bool_value_);
    }
    case NULLS: {
      return 0;  // NULL as int is 0
    }
    default: {
      LOG_WARN("unknown data type. type=%d", attr_type_);
      return 0;
    }
  }
  return 0;
}

float Value::get_float() const
{
  switch (attr_type_) {
    case CHARS:
    case TEXTS: {
      try {
        return std::stof(str_value_);
      } catch (std::exception const &ex) {
        LOG_TRACE("failed to convert string to float. s=%s, ex=%s", str_value_.c_str(), ex.what());
        return 0.0;
      }
    } break;
    case INTS: {
      return float(num_value_.int_value_);
    } break;
    case FLOATS: {
      return num_value_.float_value_;
    } break;
    case DATES: {
   return float(num_value_.date_value_);
    }  break;
    case BOOLEANS: {
      return float(num_value_.bool_value_);
    } break;
    case NULLS: {
      return 0.0;  // NULL as float is 0
    } break;
    default: {
      LOG_WARN("unknown data type. type=%d", attr_type_);
      return 0;
    }
  }
  return 0;
}

double Value::get_double() const
{
  switch (attr_type_) {
    case CHARS:
    case TEXTS: {
      try {
        return std::stod(str_value_);
      } catch (std::exception const &ex) {
        LOG_TRACE("failed to convert string to double. s=%s, ex=%s", str_value_.c_str(), ex.what());
        return 0.0;
      }
    } break;
    case INTS: {
      return static_cast<double>(num_value_.int_value_);
    } break;
    case FLOATS: {
      if (has_double_value_) {
        return double_value_;
      }
      double v = num_value_.float_value_;
      return plain_float_format_ ? std::round(v * 100.0) / 100.0 : v;
    } break;
    case DATES: {
      return static_cast<double>(num_value_.date_value_);
    } break;
    case BOOLEANS: {
      return static_cast<double>(num_value_.bool_value_);
    } break;
    case NULLS: {
      return 0.0;
    } break;
    default: {
      LOG_WARN("unknown data type. type=%d", attr_type_);
      return 0.0;
    }
  }
  return 0.0;
}

std::string Value::get_string() const
{
  if (is_null_) {
    return "NULL";
  }
  return this->to_string();
}

bool Value::get_boolean() const
{
  if (is_null_) {
    return false;  // NULL is falsy in boolean context
  }
  switch (attr_type_) {
    case CHARS:
    case TEXTS: {
      try {
        float val = std::stof(str_value_);
        if (val >= EPSILON || val <= -EPSILON) {
          return true;
        }

        int int_val = std::stol(str_value_);
        if (int_val != 0) {
          return true;
        }

        return !str_value_.empty();
      } catch (std::exception const &ex) {
        LOG_TRACE("failed to convert string to float or integer. s=%s, ex=%s", str_value_.c_str(), ex.what());
        return !str_value_.empty();
      }
    } break;
    case INTS: {
      return num_value_.int_value_ != 0;
    } break;
    case FLOATS: {
      float val = num_value_.float_value_;
      return val >= EPSILON || val <= -EPSILON;
    } break;
    case BOOLEANS: {
      return num_value_.bool_value_;
    } break;
    case NULLS: {
      return false;  // NULL is falsy
    } break;
    default: {
      LOG_WARN("unknown data type. type=%d", attr_type_);
      return false;
    }
  }
  return false;
}

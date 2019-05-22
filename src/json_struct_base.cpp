#include "json_struct_base.h"
#include "cJSON.h"
#include <map>
#include <codecvt>
#include <typeinfo>
#include <Windows.h>
#include <algorithm>

#include <boost/lexical_cast.hpp>
#include <boost/xpressive/xpressive.hpp>

using namespace boost::xpressive;

enum type {
  enum_no,

  enum_bool,

  enum_number,
  enum_number_array,

  enum_wchar_array,
  enum_wchar_table,

  enum_user_def_struct,
  enum_user_def_struct_array,
};

struct field_info {
  type type_;
  std::string name_;
  void *address_;
  int table_row_;
  int table_col_;
  const type_info *field_type_;

  field_info() {
    ZeroMemory(this, sizeof(*this));
}};

struct data_type_info {
  std::map < type, const char *>data_type_regex;

  data_type_info() {
    data_type_regex[enum_bool] = "bool";

    data_type_regex[enum_number] =
      "(?:int|unsigned short|unsigned int|long|unsigned long|__int64|float|double)";
    data_type_regex[enum_number_array] =
      "(?:int|unsigned short|unsigned int|long|unsigned long|__int64|float|double) \\[(\\d+)\\]";

    data_type_regex[enum_wchar_array] = "wchar_t \\[(\\d+)\\]";
    data_type_regex[enum_wchar_table] = "wchar_t \\[(\\d+)\\]\\[(\\d+)\\]";

    data_type_regex[enum_user_def_struct] = "struct \\w+";
    data_type_regex[enum_user_def_struct_array] = "struct \\w+\\[(\\d+)\\]";
}};

/************************************************************************/
/* save your c++ struct size information */
/* key is struct name, value is struct size */
/************************************************************************/
static std::map < std::string, int >struct_object_size_info;

/************************************************************************/
/* data type regular expressions */
/************************************************************************/
static data_type_info data_type_infos;

static int array_size(const type_info * ptype_info) {
  static cregex pattern_number_array =
    cregex::compile(data_type_infos.data_type_regex[enum_number_array]);
  static cregex pattern_number_wchar_array =
    cregex::compile(data_type_infos.data_type_regex[enum_wchar_array]);
  static cregex pattern_user_def_struct_array =
    cregex::compile(data_type_infos.
                    data_type_regex[enum_user_def_struct_array]);

  cmatch array_info;

  if (regex_match(ptype_info->name(), array_info, pattern_number_array)) {
    return atoi(array_info[1].str().c_str());
  } else
    if (regex_match
        (ptype_info->name(), array_info, pattern_number_wchar_array)) {
    return atoi(array_info[1].str().c_str());
  } else if (regex_match(ptype_info->name(), array_info, pattern_number_array)) {
    return atoi(array_info[1].str().c_str());
  }

  return 0;
}

void table_size(const type_info * ptype_info, int &row, int &col) {
  static cregex pattern =
    cregex::compile(data_type_infos.data_type_regex[enum_wchar_table]);

  cmatch table_info;

  if (regex_match(ptype_info->name(), table_info, pattern)) {
    row = atoi(table_info[1].str().c_str());
    col = atoi(table_info[2].str().c_str());
  }
}

static bool is_bool(const type_info * ptype_info) {
  static cregex pattern =
    cregex::compile(data_type_infos.data_type_regex[enum_bool]);

  return regex_match(ptype_info->name(), pattern);
}

static bool is_number(const type_info * ptype_info) {
  static cregex pattern =
    cregex::compile(data_type_infos.data_type_regex[enum_number]);

  return regex_match(ptype_info->name(), pattern);
}

static bool is_number_array(const type_info * ptype_info) {
  static cregex pattern =
    cregex::compile(data_type_infos.data_type_regex[enum_number_array]);

  return regex_match(ptype_info->name(), pattern);
}

static bool is_wchar_array(const type_info * ptype_info) {
  static cregex pattern =
    cregex::compile(data_type_infos.data_type_regex[enum_wchar_array]);

  return regex_match(ptype_info->name(), pattern);
}

static bool is_wchar_table(const type_info * ptype_info) {
  static cregex pattern =
    cregex::compile(data_type_infos.data_type_regex[enum_wchar_table]);

  return regex_match(ptype_info->name(), pattern);
}

static bool is_user_defined_struct(const type_info * ptype_info) {
  static cregex pattern =
    cregex::compile(data_type_infos.data_type_regex[enum_user_def_struct]);

  return regex_match(ptype_info->name(), pattern);
}

static bool is_user_defined_struct_array(const type_info * ptype_info) {
  static cregex pattern =
    cregex::compile(data_type_infos.
                    data_type_regex[enum_user_def_struct_array]);

  return regex_match(ptype_info->name(), pattern);
}

static type data_type(const type_info * ptype_info, field_info * pfield_info =
                      nullptr) {
  if (is_bool(ptype_info)) {
    return enum_bool;
  } else if (is_number(ptype_info)) {
    return enum_number;
  } else if (is_number_array(ptype_info)) {
    return enum_number_array;
  } else if (is_wchar_array(ptype_info)) {
    return enum_wchar_array;
  } else if (is_wchar_table(ptype_info)) {
    if (pfield_info) {
      table_size(ptype_info, pfield_info->table_row_, pfield_info->table_col_);
    }
    return enum_wchar_table;
  } else if (is_user_defined_struct(ptype_info)) {
    return enum_user_def_struct;
  } else if (is_user_defined_struct_array(ptype_info)) {
    return enum_user_def_struct_array;
  }

  return enum_no;
}

void from_number(const type_info * field_type, void *field_address,
                 cJSON * item, int offset) {
  if (typeid(int) == *field_type) {
    *((int *)field_address + offset) = item->valuedouble;
  }
  if (typeid(unsigned int) == *field_type) {
    *((unsigned int *)field_address + offset) = item->valuedouble;
  } else if (typeid(__int64) == *field_type) {
    *((__int64 *) field_address + offset) = item->valuedouble;
  } else if (typeid(long) == *field_type) {
    *((long *)field_address + offset) = item->valuedouble;
  } else if (typeid(unsigned short) == *field_type) {
    *((unsigned short *)field_address + offset) = item->valuedouble;
  } else if (typeid(unsigned long) == *field_type) {
    *((unsigned long *)field_address + offset) = item->valuedouble;
  } else if (typeid(float) == *field_type) {
    *((float *)field_address + offset) = item->valuedouble;
  } else if (typeid(double) == *field_type) {
    *((double *)field_address + offset) = item->valuedouble;
  }
}

bool json_struct_base::from_json(std::string json) {
  if (json.empty())
    return false;

  cJSON *root = cJSON_Parse(json.c_str());

  bool success = from_json_object(root);

  cJSON_Delete(root);

  return success;
}

bool json_struct_base::from_json_object(cJSON * object) {
  if (nullptr == object)
    return false;

  for (auto iter = fields_info.begin(); iter != fields_info.end(); ++iter) {
    field_info *field_information = *iter;
    void *field_address = field_information->address_;

    cJSON *item =
      cJSON_GetObjectItem(object, field_information->name_.c_str());

    if (nullptr == item)
      return false;

    switch (field_information->type_) {
    case enum_bool:
      {
        if (cJSON_False != item->type && cJSON_True != item->type)
          return false;

        *(bool *) field_address = 1 == item->valueint ? true : false;
      }
      break;
    case enum_number:
      {
        if (cJSON_Number != item->type)
          return false;

        from_number(field_information->field_type_, field_address, item, 0);
      }
      break;
    case enum_number_array:
      {
        if (cJSON_Array != item->type)
          return false;

        int arrSize1 = cJSON_GetArraySize(item);
        int arrSize2 = array_size(field_information->field_type_);

        for (int i = 0; i < arrSize1 && i < arrSize2; ++i) {
          cJSON *arrItem = cJSON_GetArrayItem(item, i);

          if (cJSON_Number != arrItem->type)
            return false;

          from_number(field_information->field_type_, field_address, item, i);
        }
      }
      break;
    case enum_wchar_array:
      {
        if (cJSON_String != item->type)
          return false;

        std::wstring ucs2 =
          std::wstring_convert < std::codecvt_utf8 < wchar_t >,
          wchar_t > ().from_bytes(item->valuestring);

        WCHAR *dst = (WCHAR *) field_address;

        wcsncpy_s(dst, array_size(field_information->field_type_) - 1,
                  ucs2.c_str(), ucs2.size());

        *(dst + ucs2.size()) = '\0';
      }
      break;
    case enum_wchar_table:
      {
        if (cJSON_Array != item->type)
          return false;

        int arrSize = cJSON_GetArraySize(item);

        for (auto i = 0; i < field_information->table_row_ && i < arrSize; ++i) {
          cJSON *arrItem = cJSON_GetArrayItem(item, i);

          if (cJSON_String != arrItem->type)
            return false;

          std::wstring ucs2 =
            std::wstring_convert < std::codecvt_utf8 < wchar_t >,
            wchar_t > ().from_bytes(arrItem->valuestring);

          WCHAR *dst =
            (WCHAR *) field_address + i * field_information->table_col_;

          wcsncpy_s(dst, field_information->table_col_ - 1, ucs2.c_str(),
                    ucs2.size());

          *(dst + ucs2.size()) = '\0';
        }
      }
      break;
    case enum_user_def_struct:
      {
        bool success =
          ((json_struct_base *) field_address)->from_json_object(item);

        if (!success)
          return false;
      }
      break;
    case enum_user_def_struct_array:
      {
        int size = 0;
        for (auto iter = struct_object_size_info.begin();
             iter != struct_object_size_info.end(); ++iter) {
          if (std::string::npos != field_information->name_.find(iter->first)) {
            size = iter->second;
            break;
          }
        }

        int arrSize = array_size(field_information->field_type_);

        for (int i = 0; i < arrSize; ++i) {
          bool success =
            ((json_struct_base *) ((BYTE *) field_address +
                                   i *
                                   size))->
            from_json_object(cJSON_GetArrayItem(item, i));

          if (!success)
            return false;
        }
      }
      break;
    case enum_no:
      return false;
    }
  }

  return true;
}

void json_struct_base::register_field(std::string st_name, int st_size,
                                      const type_info * field_type,
                                      std::string field_name,
                                      void *field_address) {
  field_info *pfield_info = new field_info;

  pfield_info->type_ = data_type(field_type);
  pfield_info->name_ = field_name;
  pfield_info->address_ = field_address;
  pfield_info->field_type_ = field_type;

  fields_info.push_back(pfield_info);

  struct_object_size_info.insert(std::make_pair(st_name, st_size));
}

json_struct_base::~json_struct_base() {
  std::for_each(fields_info.begin(), fields_info.end(),
                [&](field_info * pointer) {
                delete pointer;
                });
}
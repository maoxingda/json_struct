#include "cJSON.h"
#include "jmacro.h"
#include "jqualifier.h"
#include "jstruct_base.h"

#include <map>
#include <list>
#include <codecvt>
#include <typeinfo>
#include <Windows.h>
#include <algorithm>

#include <boost/xpressive/xpressive.hpp>

using namespace boost::xpressive;


static std::map<int, const char*>* pfield_type_re_str_ = nullptr;

enum type
{
    enum_none,

    enum_bool,

    enum_number,
    enum_number_array,

    enum_wchar_array,
    enum_wchar_table,

    enum_custom,
    enum_custom_array,
};

struct field_info
{
    int         type_;
    std::string type_name_;
    std::string qualifier_;
    std::string name_;
    std::string alias_;
    void*       address_;
    void*       address_size_;
    int         offset_;
    int         row_;
    int         col_;
};

static int array_size(const std::string& field_type)
{
    static sregex re1 = sregex::compile((*pfield_type_re_str_)[enum_number_array]);
    static sregex re2 = sregex::compile((*pfield_type_re_str_)[enum_wchar_array]);
    static sregex re3 = sregex::compile((*pfield_type_re_str_)[enum_custom_array]);

    smatch sm;

    if (regex_match(field_type, sm, re1))
    {
        return stoi(sm[1]);
    }
    else if (regex_match(field_type, sm, re2))
    {
        return stoi(sm[1]);
    }
    else if (regex_match(field_type, sm, re3))
    {
        return stoi(sm[1]);
    }

    return 0;
}

static void table_size(const std::string& field_type, int& row, int& col)
{
    static sregex re = sregex::compile((*pfield_type_re_str_)[enum_wchar_table]);

    smatch sm;

    if (regex_match(field_type, sm, re))
    {
        row = stoi(sm[1]);
        col = stoi(sm[2]);
    }
}

static bool is_bool(const std::string& field_type)
{
    static sregex re = sregex::compile((*pfield_type_re_str_)[enum_bool]);

    return regex_match(field_type, re);
}

static bool is_number(const std::string& field_type)
{
    static sregex re = sregex::compile((*pfield_type_re_str_)[enum_number]);

    return regex_match(field_type, re);
}

static bool is_number_array(const std::string& field_type)
{
    static sregex re = sregex::compile((*pfield_type_re_str_)[enum_number_array]);

    return regex_match(field_type, re);
}

static bool is_wchar_array(const std::string& field_type)
{
    static sregex re = sregex::compile((*pfield_type_re_str_)[enum_wchar_array]);

    return regex_match(field_type, re);
}

static bool is_wchar_table(const std::string& field_type)
{
    static sregex re = sregex::compile((*pfield_type_re_str_)[enum_wchar_table]);

    return regex_match(field_type, re);
}

static bool is_custom(const std::string& field_type)
{
    static sregex re = sregex::compile((*pfield_type_re_str_)[enum_custom]);

    return regex_match(field_type, re);
}

static bool is_custom_array(const std::string& field_type)
{
    static sregex re = sregex::compile((*pfield_type_re_str_)[enum_custom_array]);

    return regex_match(field_type, re);
}

static type data_type(const std::string& field_type, field_info& field_info)
{
    if (is_bool(field_type))
    {
        return enum_bool;
    }
    else if (is_number(field_type))
    {
        return enum_number;
    }
    else if (is_number_array(field_type))
    {
        field_info.col_ = array_size(field_type);

        return enum_number_array;
    }
    else if (is_wchar_array(field_type))
    {
        field_info.col_ = array_size(field_type);

        return enum_wchar_array;
    }
    else if (is_wchar_table(field_type))
    {
        table_size(field_type, field_info.row_, field_info.col_);

        return enum_wchar_table;
    }
    else if (is_custom(field_type))
    {
        return enum_custom;
    }
    else if (is_custom_array(field_type))
    {
        field_info.col_ = array_size(field_type);

        return enum_custom_array;
    }

    return enum_none;
}

static void from_number(const std::string& field_type, void* field_address, cJSON* item)
{
    if (typeid(int).name() == field_type)
    {
        *((int*)field_address) = item->valueint;
    }
    else if (typeid(unsigned int).name() == field_type)
    {
        if (UINT_MAX <= item->valuedouble)
        {
            *((unsigned int*)field_address) = UINT_MAX;
        }
        else if ((double)0 >= item->valuedouble)
        {
            *((unsigned int*)field_address) = (unsigned int)0;
        }
        else
        {
            *((unsigned int*)field_address) = (unsigned int)item->valuedouble;
        }
    }
    else if (typeid(__int64).name() == field_type)
    {
        if (INT64_MAX <= item->valuedouble)
        {
            *((__int64*)field_address) = INT64_MAX;
        }
        else if ((double)INT64_MIN >= item->valuedouble)
        {
            *((__int64*)field_address) = (__int64)INT64_MIN;
        }
        else
        {
            *((__int64*)field_address) = (__int64)item->valuedouble;
        }
    }
    else if (typeid(long).name() == field_type)
    {
        if (LONG_MAX <= item->valuedouble)
        {
            *((long*)field_address) = LONG_MAX;
        }
        else if ((double)LONG_MIN >= item->valuedouble)
        {
            *((long*)field_address) = LONG_MIN;
        }
        else
        {
            *((long*)field_address) = (long)item->valuedouble;
        }
    }
    else if (typeid(unsigned short).name() == field_type)
    {
        if (0xffff <= item->valuedouble)
        {
            *((unsigned short*)field_address) = (unsigned short)0xffff;
        }
        else if ((double)0 >= item->valuedouble)
        {
            *((unsigned short*)field_address) = (unsigned short)0;
        }
        else
        {
            *((unsigned short*)field_address) = (unsigned short)item->valuedouble;
        }
    }
    else if (typeid(unsigned long).name() == field_type)
    {
        if (ULONG_MAX <= item->valuedouble)
        {
            *((unsigned long*)field_address) = ULONG_MAX;
        }
        else if ((double)0 >= item->valuedouble)
        {
            *((unsigned long*)field_address) = (unsigned long)0;
        }
        else
        {
            *((unsigned long*)field_address) = (unsigned long)item->valuedouble;
        }
    }
    else if (typeid(float).name() == field_type)
    {
        *((float*)field_address) = (float)item->valuedouble;
    }
    else if (typeid(double).name() == field_type)
    {
        *((double*)field_address) = item->valuedouble;
    }
}

static void from_number_array(const std::string& field_type, void* field_address, cJSON* item, int offset)
{
    if (std::string::npos != std::string(field_type).find("int"))
    {
        *((int*)field_address + offset) = item->valueint;
    }
    else if (std::string::npos != std::string(field_type).find("unsigned int"))
    {
        if (UINT_MAX <= item->valuedouble)
        {
            *((unsigned int*)field_address + offset) = UINT_MAX;
        }
        else if ((double)0 >= item->valuedouble)
        {
            *((unsigned int*)field_address + offset) = (unsigned int)0;
        }
        else
        {
            *((unsigned int*)field_address + offset) = (unsigned int)item->valuedouble;
        }
    }
    else if (std::string::npos != std::string(field_type).find("__int64"))
    {
        if (INT64_MAX <= item->valuedouble)
        {
            *((__int64*)field_address + offset) = INT64_MAX;
        }
        else if ((double)INT64_MIN >= item->valuedouble)
        {
            *((__int64*)field_address + offset) = (__int64)INT64_MIN;
        }
        else
        {
            *((__int64*)field_address + offset) = (__int64)item->valuedouble;
        }
    }
    else if (std::string::npos != std::string(field_type).find("long"))
    {
        if (LONG_MAX <= item->valuedouble)
        {
            *((long*)field_address + offset) = LONG_MAX;
        }
        else if ((double)LONG_MIN >= item->valuedouble)
        {
            *((long*)field_address + offset) = LONG_MIN;
        }
        else
        {
            *((long*)field_address + offset) = (long)item->valuedouble;
        }
    }
    else if (std::string::npos != std::string(field_type).find("unsigned short"))
    {
        if (0xffff <= item->valuedouble)
        {
            *((unsigned short*)field_address + offset) = (unsigned short)0xffff;
        }
        else if ((double)0 >= item->valuedouble)
        {
            *((unsigned short*)field_address + offset) = (unsigned short)0;
        }
        else
        {
            *((unsigned short*)field_address + offset) = (unsigned short)item->valuedouble;
        }
    }
    else if (std::string::npos != std::string(field_type).find("unsigned long"))
    {
        if (ULONG_MAX <= item->valuedouble)
        {
            *((unsigned long*)field_address + offset) = ULONG_MAX;
        }
        else if ((double)0 >= item->valuedouble)
        {
            *((unsigned long*)field_address + offset) = (unsigned long)0;
        }
        else
        {
            *((unsigned long*)field_address + offset) = (unsigned long)item->valuedouble;
        }
    }
    else if (std::string::npos != std::string(field_type).find("float"))
    {
        *((float*)field_address + offset) = (float)item->valuedouble;
    }
    else if (std::string::npos != std::string(field_type).find("double"))
    {
        *((double*)field_address + offset) = item->valuedouble;
    }
}

bool jstruct_base::from_json(std::string json)
{
    if (json.empty()) return false;

    cJSON* root = cJSON_Parse(json.c_str());

    bool success = from_json_(root);

    cJSON_Delete(root);

    return success;
}

bool jstruct_base::from_json_(void* object)
{
    auto data = (std::list<field_info>*)d;

    if (nullptr == object)       return false;
    if (0       == data->size()) return false;

    for (auto iter = data->begin(); iter != data->end(); ++iter)
    {
        auto&               field_information   = *iter;
        void*               field_address       = field_information.address_;
        std::string         alias               = field_information.alias_;

        cJSON*              item                = nullptr;
        if (!alias.empty()) item                = cJSON_GetObjectItem((cJSON*)object, alias.c_str());
        if (!item)          item                = cJSON_GetObjectItem((cJSON*)object, field_information.name_.c_str());

        if (nullptr == item)
        {
            if (ESTR(OPTIONAL) == field_information.qualifier_) continue;
            if (ESTR(REQUIRED) == field_information.qualifier_) return false;
        }

        switch (field_information.type_)
        {
        case enum_bool:
            {
                if (cJSON_IsBool(item)) *(bool*)field_address = 1 == item->valueint ? true : false;
            }
            break;
        case enum_number:
            {
                if (cJSON_IsNumber(item)) from_number(field_information.type_name_, field_address, item);
            }
            break;
        case enum_number_array:
            {
                if (cJSON_IsArray(item))
                {
                    int size = cJSON_GetArraySize(item);

                    for (int i = 0; i < size && i < field_information.col_; ++i)
                    {
                        cJSON* arrItem = cJSON_GetArrayItem(item, i);

                        if (cJSON_IsNumber(arrItem)) from_number_array(field_information.type_name_, field_address, arrItem, i);
                    }

                    *((int*)field_information.address_size_) = min(size, field_information.col_);
                }
            }
            break;
        case enum_wchar_array:
            {
                if (cJSON_IsString(item))
                {
                    std::wstring ucs2 = std::wstring_convert<std::codecvt_utf8 <wchar_t>, wchar_t>().from_bytes(item->valuestring);

                    WCHAR* dst = (WCHAR*)field_address;

                    wcsncpy_s(dst, field_information.col_ - 1, ucs2.c_str(), ucs2.size());

                    *(dst + ucs2.size()) = '\0';
                }
            }
            break;
        case enum_wchar_table:
            {
                if (cJSON_IsArray(item))
                {
                    int size = cJSON_GetArraySize(item);

                    for (auto i = 0; i < field_information.row_ && i < size; ++i)
                    {
                        cJSON* arrItem = cJSON_GetArrayItem(item, i);

                        if (cJSON_IsString(arrItem))
                        {
                            std::wstring ucs2 = std::wstring_convert<std::codecvt_utf8 <wchar_t>, wchar_t>().from_bytes(arrItem->valuestring);

                            WCHAR *dst = (WCHAR *) field_address + i * field_information.col_;

                            wcsncpy_s(dst, field_information.col_ - 1, ucs2.c_str(), ucs2.size());

                            *(dst + ucs2.size()) = '\0';
                        }
                    }

                    *((int*)field_information.address_size_) = min(size, field_information.row_);
                }
            }
            break;
        case enum_custom:
            {
                if (cJSON_IsObject(item))
                {
                    bool success = ((jstruct_base *)field_address)->from_json_(item);

                    if (!success) return false;
                }
            }
            break;
        case enum_custom_array:
            {
                if (cJSON_IsArray(item))
                {
                    int size = cJSON_GetArraySize(item);

                    for (int i = 0; i < field_information.col_ && i < size; ++i)
                    {
                        cJSON* arrItem = cJSON_GetArrayItem(item, i);

                        if (cJSON_IsObject(arrItem))
                        {
                            bool success = ((jstruct_base*)((byte*)field_address + i * field_information.offset_))->from_json_(arrItem);

                            if (!success) return false;
                        }
                    }

                    *((int*)field_information.address_size_) = min(size, field_information.col_);
                }
            }
            break;
        }
    }

    return true;
}

void jstruct_base::register_field(std::string field_type, std::string field_qualifier, std::string field_name, std::string field_name_alias, void* field_address, void* array_size_field_address, int offset)
{
    field_info f_info;

    f_info.type_         = data_type(field_type, f_info);
    f_info.type_name_    = field_type;
    f_info.qualifier_    = field_qualifier;
    f_info.name_         = field_name;
    f_info.alias_        = field_name_alias;
    f_info.address_      = field_address;
    f_info.address_size_ = array_size_field_address;
    f_info.offset_       = offset;

    ((std::list<field_info>*)d)->push_back(f_info);
}

jstruct_base::jstruct_base()
    : d(new std::list<field_info>())
{
    static bool init = false;

    if (!init)
    {
        init = true;

        static std::map<int, const char*> field_type_re_str_; // field type regular expression string map

        pfield_type_re_str_                   = &field_type_re_str_;

        field_type_re_str_[enum_bool]         = "bool";

        field_type_re_str_[enum_number]       = "(?:int|unsigned short|unsigned int|long|unsigned long|__int64|float|double)";
        field_type_re_str_[enum_number_array] = "(?:int|unsigned short|unsigned int|long|unsigned long|__int64|float|double) \\[(\\d+)\\]";

        field_type_re_str_[enum_wchar_array]  = "wchar_t \\[(\\d+)\\]";
        field_type_re_str_[enum_wchar_table]  = "wchar_t \\[(\\d+)\\]\\[(\\d+)\\]";

        field_type_re_str_[enum_custom]       = "struct \\w+";
        field_type_re_str_[enum_custom_array] = "struct \\w+ \\[(\\d+)\\]";
    }
}

jstruct_base::~jstruct_base()
{
    delete ((std::list<field_info>*)d);
    d = nullptr;
}

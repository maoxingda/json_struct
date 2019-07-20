#include "cJSON.h"
#include "jmacro.h"
#include "jqualifier.h"
#include "jstruct_base.h"
#include "jfield_info.h"

#include <list>
#include <codecvt>

#include <boost/format.hpp>
#include <boost/xpressive/xpressive_static.hpp>

using namespace boost::xpressive;

// common regex expressions
static const sregex re_bool         = as_xpr("bool");

static const sregex re_array        = as_xpr("[") >> (s1 = +_d) >> "]";

static const sregex re_number       = (as_xpr("short") | "unsigned short" | "int" | "unsigned int" | "long" | "unsigned long" | "__int64" | "float" | "double");
static const sregex re_number_array = (re_number >> " " >> "[" >> (s1 = +_d) >> "]");

static const sregex re_wchar_array  = (as_xpr("wchar_t ") >> "[" >> (s1 = +_d) >> "]");
static const sregex re_wchar_table  = (as_xpr("wchar_t ") >> ("[" >> (s1 = +_d) >> "]") >> ("[" >> (s2 = +_d) >> "]"));

static const sregex re_struct       = (as_xpr("struct ") >> +_w);
static const sregex re_struct_array = (as_xpr("struct ") >> +_w >> " " >> "[" >> (s1 = +_d) >> "]");

static int array_size(const string& field_type)
{
    smatch sm;

    if (regex_match(field_type, sm, re_number_array))
    {
        return stoi(sm[s1]);
    }
    else if (regex_match(field_type, sm, re_wchar_array))
    {
        return stoi(sm[s1]);
    }
    else if (regex_match(field_type, sm, re_struct_array))
    {
        return stoi(sm[s1]);
    }

    return 0;
}

static void table_size(const string& field_type, size_t& row, size_t& col)
{
    smatch sm;

    if (regex_match(field_type, sm, re_wchar_table))
    {
        row = stoi(sm[s1]);
        col = stoi(sm[s2]);
    }
}

static bool is_bool(const string& field_type)
{
    return regex_match(field_type, re_bool);
}

static bool is_number(const string& field_type)
{
    return regex_match(field_type, re_number);
}

static bool is_number_array(const string& field_type)
{
    return regex_match(field_type, re_number_array);
}

static bool is_wchar_array(const string& field_type)
{
    return regex_match(field_type, re_wchar_array);
}

static bool is_wchar_table(const string& field_type)
{
    return regex_match(field_type, re_wchar_table);
}

static bool is_struct(const string& field_type)
{
    return regex_match(field_type, re_struct);
}

static bool is_struct_array(const string& field_type)
{
    return regex_match(field_type, re_struct_array);
}

static type data_type(const string& field_type, size_t& row, size_t& col)
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
        col = array_size(field_type);

        return enum_number_array;
    }
    else if (is_wchar_array(field_type))
    {
        col = array_size(field_type);

        return enum_wchar_array;
    }
    else if (is_wchar_table(field_type))
    {
        table_size(field_type, row, col);

        return enum_wchar_table;
    }
    else if (is_struct(field_type))
    {
        return enum_struct;
    }
    else if (is_struct_array(field_type))
    {
        col = array_size(field_type);

        return enum_struct_array;
    }

    return enum_none;
}

static cJSON* to_number(const string& field_type, const string& field_name, void* field_address, cJSON* object)
{
    if (typeid(int).name() == field_type)
    {
        return cJSON_AddNumberToObject(object, field_name.c_str(), *(int*)field_address);
    }
    else if (typeid(unsigned int).name() == field_type)
    {
        return cJSON_AddNumberToObject(object, field_name.c_str(), *(unsigned int*)field_address);
    }
    else if (typeid(__int64).name() == field_type)
    {
        return cJSON_AddNumberToObject(object, field_name.c_str(), *(double*)field_address);
    }
    else if (typeid(long).name() == field_type)
    {
        return cJSON_AddNumberToObject(object, field_name.c_str(), *(long*)field_address);
    }
    else if (typeid(unsigned short).name() == field_type)
    {
        return cJSON_AddNumberToObject(object, field_name.c_str(), *(unsigned short*)field_address);
    }
    else if (typeid(unsigned long).name() == field_type)
    {
        return cJSON_AddNumberToObject(object, field_name.c_str(), *(unsigned long*)field_address);
    }
    else if (typeid(float).name() == field_type)
    {
        return cJSON_AddNumberToObject(object, field_name.c_str(), *(float*)field_address);
    }
    else if (typeid(double).name() == field_type)
    {
        return cJSON_AddNumberToObject(object, field_name.c_str(), *(double*)field_address);
    }

    return nullptr;
}

static bool to_number_array(const string& field_type, void* field_address, cJSON* array, int offset)
{
    if (std::string::npos != string(field_type).find("int"))
    {
        cJSON* item = cJSON_CreateNumber(*((int*)field_address + offset));

        if (nullptr == item) return false;

        cJSON_AddItemToArray(array, item);
    }
    else if (std::string::npos != string(field_type).find("unsigned int"))
    {
        cJSON* item = cJSON_CreateNumber(*((unsigned int*)field_address + offset));

        if (nullptr == item) return false;

        cJSON_AddItemToArray(array, item);
    }
    else if (std::string::npos != string(field_type).find("__int64"))
    {
        cJSON* item = cJSON_CreateNumber(*((double*)field_address + offset));

        if (nullptr == item) return false;

        cJSON_AddItemToArray(array, item);
    }
    else if (std::string::npos != string(field_type).find("long"))
    {
        cJSON* item = cJSON_CreateNumber(*((long*)field_address + offset));

        if (nullptr == item) return false;

        cJSON_AddItemToArray(array, item);
    }
    else if (std::string::npos != string(field_type).find("unsigned short"))
    {
        cJSON* item = cJSON_CreateNumber(*((unsigned short*)field_address + offset));

        if (nullptr == item) return false;

        cJSON_AddItemToArray(array, item);
    }
    else if (std::string::npos != string(field_type).find("unsigned long"))
    {
        cJSON* item = cJSON_CreateNumber(*((unsigned long*)field_address + offset));

        if (nullptr == item) return false;

        cJSON_AddItemToArray(array, item);
    }
    else if (std::string::npos != string(field_type).find("float"))
    {
        cJSON* item = cJSON_CreateNumber(*((float*)field_address + offset));

        if (nullptr == item) return false;

        cJSON_AddItemToArray(array, item);
    }
    else if (std::string::npos != string(field_type).find("double"))
    {
        cJSON* item = cJSON_CreateNumber(*((double*)field_address + offset));

        if (nullptr == item) return false;

        cJSON_AddItemToArray(array, item);
    }

    return true;
}

static void from_number(const string& field_type, void* field_address, cJSON* item)
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

static void from_number_array(const string& field_type, void* field_address, cJSON* item, int offset)
{
    if (string::npos != string(field_type).find("int"))
    {
        *((int*)field_address + offset) = item->valueint;
    }
    else if (string::npos != string(field_type).find("unsigned int"))
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
    else if (string::npos != string(field_type).find("__int64"))
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
    else if (string::npos != string(field_type).find("long"))
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
    else if (string::npos != string(field_type).find("unsigned short"))
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
    else if (string::npos != string(field_type).find("unsigned long"))
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
    else if (string::npos != string(field_type).find("float"))
    {
        *((float*)field_address + offset) = (float)item->valuedouble;
    }
    else if (string::npos != string(field_type).find("double"))
    {
        *((double*)field_address + offset) = item->valuedouble;
    }
}

string jstruct_base::to_json()
{
    string json;

    bool success = true;

    cJSON* object = (cJSON*)to_json_(success);

    if (!success)
    {
        return json;
    }

    char* s = cJSON_PrintUnformatted(object);

    json = s;

    delete s;

    cJSON_Delete(object);

    return json;
}

void* jstruct_base::to_json_(bool& success)
{
    auto data = (list<field_info>*)d;

    if (nullptr == data)         return nullptr;
    if (0       == data->size()) return nullptr;

    cJSON* object = cJSON_CreateObject();

    if (nullptr == object)
    {
        success = false;

        return nullptr;
    }

    for (auto iter = data->begin(); iter != data->end(); ++iter)
    {
        auto&  field_information = *iter;
        void*  field_address     = field_information.address_;
        string field_name        = field_information.alias_.empty() ? field_information.name_.c_str() : field_information.alias_.c_str();

        switch (field_information.type_)
        {
        case enum_bool:
            {
                cJSON* item = cJSON_AddBoolToObject(object, field_name.c_str(), *(bool*)field_address);

                if (nullptr == item)
                {
                    success = false;

                    return nullptr;
                }
            }
            break;
        case enum_number:
            {
                cJSON* item = to_number(field_information.type_name_, field_name, field_address, object);

                if (nullptr == item)
                {
                    success = false;

                    return nullptr;
                }
            }
            break;
        case enum_number_array:
            {
                int size = *(int*)field_information.address_size_;

                cJSON* array = cJSON_CreateArray();

                if (nullptr == array)
                {
                    success = false;

                    return nullptr;
                }

                for (auto i = 0; i < size; ++i)
                {
                    bool success = to_number_array(field_information.type_name_, field_address, array, i * field_information.offset_);

                    if (!success)
                    {
                        success = false;

                        return nullptr;
                    }
                }

                cJSON_AddItemToObject(object, field_name.c_str(), array);
            }
            break;
        case enum_wchar_array:
            {
                string utf8 = wstring_convert<codecvt_utf8 <wchar_t>, wchar_t>().to_bytes((wchar_t*)field_address);

                cJSON* item = cJSON_AddStringToObject(object, field_name.c_str(), utf8.c_str());

                if (nullptr == item)
                {
                    success = false;

                    return nullptr;
                }
            }
            break;
        case enum_wchar_table:
            {
                int size = *(int*)field_information.address_size_;

                cJSON* array = cJSON_CreateArray();

                if (nullptr == array)
                {
                    success = false;

                    return nullptr;
                }

                for (auto i = 0; i < size; ++i)
                {
                    string utf8 = wstring_convert<codecvt_utf8 <wchar_t>, wchar_t>().to_bytes((wchar_t*)field_address + i * field_information.offset_);

                    cJSON* item = cJSON_CreateString(utf8.c_str());

                    if (nullptr == item)
                    {
                        success = false;

                        return nullptr;
                    }

                    cJSON_AddItemToArray(array, item);
                }
            }
            break;
        case enum_struct:
            {
                bool subsuccess = true;

                cJSON* subobject = (cJSON*)((jstruct_base*)field_address)->to_json_(subsuccess);

                if (!subsuccess)
                {
                    cJSON_Delete(subobject);

                    return nullptr;
                }

                cJSON_AddItemToObject(object, field_name.c_str(), subobject);
            }
            break;
        case enum_struct_array:
            {
                int size = *(int*)field_information.address_size_;

                cJSON* array = cJSON_CreateArray();

                if (nullptr == array)
                {
                    success = false;

                    return nullptr;
                }

                for (auto i = 0; i < size; ++i)
                {
                    bool subsuccess = true;

                    cJSON* subobject = (cJSON*)((jstruct_base*)field_address + i * field_information.offset_)->to_json_(subsuccess);

                    if (!subsuccess)
                    {
                        cJSON_Delete(subobject);

                        return nullptr;
                    }

                    cJSON_AddItemToObject(object, field_name.c_str(), subobject);
                }
            }
            break;
        }
    }

    return object;
}

bool jstruct_base::from_json(string json)
{
    if (json.empty()) return false;

    cJSON* root = cJSON_Parse(json.c_str());

    bool success = from_json_(root);

    cJSON_Delete(root);

    return success;
}

bool jstruct_base::from_json_(void* object)
{
    auto data = (list<field_info>*)d;

    if (nullptr == data)         return false;
    if (nullptr == object)       return false;
    if (0       == data->size()) return false;

    for (auto iter = data->begin(); iter != data->end(); ++iter)
    {
        auto&               field_information   = *iter;
        void*               field_address       = field_information.address_;
        string              alias               = field_information.alias_;

        cJSON*              item                = nullptr;
        if (!alias.empty()) item                = cJSON_GetObjectItem((cJSON*)object, alias.c_str());
        if (!item)          item                = cJSON_GetObjectItem((cJSON*)object, field_information.name_.c_str());

        if (nullptr == item)
        {
            if (ESTR(OPTIONAL) == field_information.qualifier_) continue;
            if (ESTR(REQUIRED) == field_information.qualifier_)
            {
#ifdef _DEBUG
                throw logic_error(field_information.name_);
#endif // _DEBUG

                return false;
            }
        }

        switch (field_information.type_)
        {
        case enum_bool:
            {
                if (cJSON_IsBool(item))
                {
                    *(bool*)field_address = 1 == item->valueint ? true : false;
                }
#ifdef _DEBUG
                else
                {
                    throw logic_error(field_information.name_);
                }
#endif // _DEBUG
            }
            break;
        case enum_number:
            {
                if (cJSON_IsNumber(item))
                {
                    from_number(field_information.type_name_, field_address, item);
                }
#ifdef _DEBUG
                else
                {
                    throw logic_error(field_information.name_);
                }
#endif // _DEBUG
            }
            break;
        case enum_number_array:
            {
                if (cJSON_IsArray(item))
                {
                    size_t size = cJSON_GetArraySize(item);

                    for (auto i = 0u; i < size && i < field_information.col_; ++i)
                    {
                        cJSON* arrItem = cJSON_GetArrayItem(item, i);

                        if (cJSON_IsNumber(arrItem)) from_number_array(field_information.type_name_, field_address, arrItem, i);
                    }

                    *((int*)field_information.address_size_) = min(size, field_information.col_);
                }
#ifdef _DEBUG
                else
                {
                    throw logic_error(field_information.name_);
                }
#endif // _DEBUG
            }
            break;
        case enum_wchar_array:
            {
                if (cJSON_IsString(item))
                {
                    wstring ucs2 = wstring_convert<codecvt_utf8 <wchar_t>, wchar_t>().from_bytes(item->valuestring);

                    wchar_t* dst = (wchar_t*)field_address;

                    wcsncpy_s(dst, field_information.col_ - 1, ucs2.c_str(), ucs2.size());
                }
#ifdef _DEBUG
                else
                {
                    throw logic_error(field_information.name_);
                }
#endif // _DEBUG
            }
            break;
        case enum_wchar_table:
            {
                if (cJSON_IsArray(item))
                {
                    size_t size = cJSON_GetArraySize(item);

                    for (auto i = 0u; i < field_information.row_ && i < size; ++i)
                    {
                        cJSON* arrItem = cJSON_GetArrayItem(item, i);

                        if (cJSON_IsString(arrItem))
                        {
                            wstring ucs2 = wstring_convert<codecvt_utf8 <wchar_t>, wchar_t>().from_bytes(arrItem->valuestring);

                            wchar_t *dst = (wchar_t*) field_address + i * field_information.col_;

                            wcsncpy_s(dst, field_information.col_ - 1, ucs2.c_str(), ucs2.size());
                        }
                    }

                    *((int*)field_information.address_size_) = min(size, field_information.row_);
                }
#ifdef _DEBUG
                else
                {
                    throw logic_error(field_information.name_);
                }
#endif // _DEBUG
            }
            break;
        case enum_struct:
            {
                if (cJSON_IsObject(item))
                {
                    bool success = ((jstruct_base*)field_address)->from_json_(item);

                    if (!success) return false;
                }
#ifdef _DEBUG
                else
                {
                    throw logic_error(field_information.name_);
                }
#endif // _DEBUG
            }
            break;
        case enum_struct_array:
            {
                if (cJSON_IsArray(item))
                {
                    size_t size = cJSON_GetArraySize(item);

                    for (auto i = 0u; i < field_information.col_ && i < size; ++i)
                    {
                        cJSON* arrItem = cJSON_GetArrayItem(item, i);

                        if (cJSON_IsObject(arrItem))
                        {
                            bool success = ((jstruct_base*)((char*)field_address + i * field_information.offset_))->from_json_(arrItem);

                            if (!success) return false;
                        }
                    }

                    *((int*)field_information.address_size_) = min(size, field_information.col_);
                }
#ifdef _DEBUG
                else
                {
                    throw logic_error(field_information.name_);
                }
#endif // _DEBUG
            }
            break;
        }
    }

    return true;
}

void jstruct_base::register_field(string field_type, string field_qualifier, string field_name, string field_name_alias, void* field_address, void* array_size_field_address, int offset)
{
    field_info f_info;
    size_t row = 0, col = 0;

    f_info.type_name_    = field_type;
    f_info.qualifier_    = field_qualifier;
    f_info.name_         = field_name;
    f_info.alias_        = field_name_alias;
    f_info.address_      = field_address;
    f_info.address_size_ = array_size_field_address;
    f_info.type_         = data_type(field_type, row, col);
    f_info.offset_       = offset;
    f_info.row_          = row;
    f_info.col_          = col;

    ((list<field_info>*)d)->push_back(f_info);
}

jstruct_base::jstruct_base()
    : d(new list<field_info>())
{
}

jstruct_base::jstruct_base(const jstruct_base& other)
    : d(nullptr)
{
}

const jstruct_base& jstruct_base::operator=(const jstruct_base& other)
{
    return *this;
}

jstruct_base::~jstruct_base()
{
    delete ((list<field_info>*)d);
    d = nullptr;
}

#include "stdafx.h"
#include "cJSON.h"
#include "jmacro.h"
#include "jqualifier.h"
#include "jstruct_base.h"
#include "jfield_info.h"
#include "jutil_common_path.h"

#include <list>
#include <codecvt>

#include <boost/format.hpp>
#include <boost/xpressive/xpressive_static.hpp>

using namespace boost::xpressive;

// common regex expressions
static const sregex re_bool          = as_xpr("bool");

static const sregex re_number        = (as_xpr("int") | "unsigned int" | "__int64" | "unsigned __int64" | "float" | "double");
static const sregex re_number_array1 = (re_number >> " " >> "[" >> (s1 = +_d) >> "]");
static const sregex re_number_array2 = (s1 = re_number) >> " " >> "[" >> +_d >> "]";

static const sregex re_wchar_array   = (as_xpr("wchar_t ") >> "[" >> (s1 = +_d) >> "]");
static const sregex re_wchar_table   = (as_xpr("wchar_t ") >> ("[" >> (s1 = +_d) >> "]") >> ("[" >> (s2 = +_d) >> "]"));

static const sregex re_struct        = (as_xpr("struct ") >> +_w >> *("::" >> +_w));
static const sregex re_struct_array  = (as_xpr("struct ") >> +_w >> *("::" >> +_w) >> " " >> "[" >> (s1 = +_d) >> "]");


static int array_size(const string& field_type)
{
    smatch sm;

    if (regex_match(field_type, sm, re_number_array1))
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
    return regex_match(field_type, re_number_array1);
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
        return cJSON_AddNumberToObject(object, field_name.c_str(), (double)(*(__int64*)field_address));
    }
    else if (typeid(unsigned __int64).name() == field_type)
    {
        return cJSON_AddNumberToObject(object, field_name.c_str(), (double)(*(unsigned __int64*)field_address));
    }
    else if (typeid(float).name() == field_type)
    {
        return cJSON_AddNumberToObject(object, field_name.c_str(), (double)*(float*)field_address);
    }
    else if (typeid(double).name() == field_type)
    {
        return cJSON_AddNumberToObject(object, field_name.c_str(), (double)*(double*)field_address);
    }

    return nullptr;
}

static bool to_number_array(const string& field_type, void* field_address, cJSON* array, int offset)
{
    smatch what;

    regex_match(field_type, what, re_number_array2);

    if ("int" == what[s1])
    {
        cJSON* item = cJSON_CreateNumber(*((int*)field_address + offset));

        if (nullptr == item) return false;

        cJSON_AddItemToArray(array, item);
    }
    else if ("unsigned int" == what[s1])
    {
        cJSON* item = cJSON_CreateNumber(*((unsigned int*)field_address + offset));

        if (nullptr == item) return false;

        cJSON_AddItemToArray(array, item);
    }
    else if ("__int64" == what[s1])
    {
        cJSON* item = cJSON_CreateNumber((double)(*((__int64*)field_address + offset)));

        if (nullptr == item) return false;

        cJSON_AddItemToArray(array, item);
    }
    else if ("unsigned __int64" == what[s1])
    {
        cJSON* item = cJSON_CreateNumber((double)(*((unsigned __int64*)field_address + offset)));

        if (nullptr == item) return false;

        cJSON_AddItemToArray(array, item);
    }
    else if ("float" == what[s1])
    {
        cJSON* item = cJSON_CreateNumber(*((float*)field_address + offset));

        if (nullptr == item) return false;

        cJSON_AddItemToArray(array, item);
    }
    else if ("double" == what[s1])
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
        auto& field = *((unsigned int*)field_address);

        if (UINT_MAX < item->valuedouble)
        {
            field = UINT_MAX;
        }
        else if (0.0 > item->valuedouble)
        {
            field = 0u;
        }
        else
        {
            field = (unsigned int)item->valuedouble;
        }
    }
    else if (typeid(__int64).name() == field_type)
    {
        auto& field = *((__int64*)field_address);

        if (INT64_MAX < item->valuedouble)
        {
            field = INT64_MAX;
        }
        else if ((double)INT64_MIN > item->valuedouble)
        {
            field = INT64_MIN;
        }
        else
        {
            field = (__int64)item->valuedouble;
        }
    }
    else if (typeid(unsigned __int64).name() == field_type)
    {
        auto& field = *((unsigned __int64*)field_address);

        if (UINT64_MAX < item->valuedouble)
        {
            field = UINT64_MAX;
        }
        else if (0.0 > item->valuedouble)
        {
            field = 0u;
        }
        else
        {
            field = (unsigned __int64)item->valuedouble;
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
    smatch what;

    regex_match(field_type, what, re_number_array2);

    if ("int" == what[s1])
    {
        *((int*)field_address + offset) = item->valueint;
    }
    else if ("unsigned int" == what[s1])
    {
        auto& field = *((unsigned int*)field_address + offset);

        if (UINT_MAX < item->valuedouble)
        {
            field = UINT_MAX;
        }
        else if (0.0 > item->valuedouble)
        {
            field = 0u;
        }
        else
        {
            field = (unsigned int)item->valuedouble;
        }
    }
    else if ("__int64" == what[s1])
    {
        auto& field = *((__int64*)field_address + offset);

        if (INT64_MAX < item->valuedouble)
        {
            field = INT64_MAX;
        }
        else if ((double)INT64_MIN > item->valuedouble)
        {
            field = INT64_MIN;
        }
        else
        {
            field = (__int64)item->valuedouble;
        }
    }
    else if ("unsigned __int64" == what[s1])
    {
        auto& field = *((__int64*)field_address + offset);

        if (UINT64_MAX < item->valuedouble)
        {
            field = UINT64_MAX;
        }
        else if (0.0 > item->valuedouble)
        {
            field = 0u;
        }
        else
        {
            field = (unsigned __int64)item->valuedouble;
        }
    }
    else if ("float" == what[s1])
    {
        *((float*)field_address + offset) = (float)item->valuedouble;
    }
    else if ("double" == what[s1])
    {
        *((double*)field_address + offset) = item->valuedouble;
    }
}

#ifdef _DEBUG
#include <fstream>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

namespace pt = boost::property_tree;


struct debug_conf
{
    bool throw_;

    void load(const std::string &filename)
    {
        pt::ptree tree;

        pt::read_xml(filename, tree);

        throw_ = tree.get<bool>("debug.throw");
    }
};

#endif // _DEBUG

static void report_error(string msg)
{
#ifdef _DEBUG
    debug_conf conf;

    conf.load(jutil_common_path().my_documents() + "\\Visual Studio 2010\\Addins\\debugconf.xml");

    conf.throw_ ? throw logic_error(msg) : 0;
#endif // _DEBUG
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
                    bool success = to_number_array(field_information.type_name_, field_address, array, i);

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
                    string utf8 = wstring_convert<codecvt_utf8 <wchar_t>, wchar_t>().to_bytes((wchar_t*)field_address + i * field_information.offset_ / 2);

                    cJSON* item = cJSON_CreateString(utf8.c_str());

                    if (nullptr == item)
                    {
                        success = false;

                        return nullptr;
                    }

                    cJSON_AddItemToArray(array, item);
                }

                cJSON_AddItemToObject(object, field_name.c_str(), array);
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

                    cJSON* arrItem = (cJSON*)((jstruct_base*)((char*)field_address + i * field_information.offset_))->to_json_(subsuccess);

                    if (!subsuccess)
                    {
                        cJSON_Delete(arrItem);

                        return nullptr;
                    }

                    cJSON_AddItemToArray(array, arrItem);
                }

                cJSON_AddItemToObject(object, field_name.c_str(), array);
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

        cJSON*              item                = cJSON_GetObjectItem((cJSON*)object, field_information.name_.c_str());

        if (nullptr == item)
        {
            if (ESTR(jopt) == field_information.qualifier_) continue;
            if (ESTR(jreq) == field_information.qualifier_)
            {
                report_error("missing required field ---> " + field_information.name_);

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
                else
                {
                    report_error("expect bool type ---> " + field_information.name_);
                }
            }
            break;
        case enum_number:
            {
                if (cJSON_IsNumber(item))
                {
                    from_number(field_information.type_name_, field_address, item);
                }
                else
                {
                    report_error("expect number type ---> " + field_information.name_);
                }
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

                        if (cJSON_IsNumber(arrItem))
                        {
                            from_number_array(field_information.type_name_, field_address, arrItem, i);
                        }
                        else
                        {
                            report_error("expect number type ---> " + field_information.name_);
                        }
                    }

                    *((int*)field_information.address_size_) = min(size, field_information.col_);
                }
                else
                {
                    report_error("expect array type ---> " + field_information.name_);
                }
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
                else
                {
                    report_error("expect string type ---> " + field_information.name_);
                }
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
                        else
                        {
                            report_error("expect string type ---> " + field_information.name_);
                        }
                    }

                    *((int*)field_information.address_size_) = min(size, field_information.row_);
                }
                else
                {
                    report_error("expect array type ---> " + field_information.name_);
                }
            }
            break;
        case enum_struct:
            {
                if (cJSON_IsObject(item))
                {
                    bool success = ((jstruct_base*)field_address)->from_json_(item);

                    if (!success) return false;
                }
                else
                {
                    report_error("expect struct type ---> " + field_information.name_);
                }
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
                        else
                        {
                            report_error("expect object type ---> " + field_information.name_);
                        }
                    }

                    *((int*)field_information.address_size_) = min(size, field_information.col_);
                }
                else
                {
                    report_error("expect array type ---> " + field_information.name_);
                }
            }
            break;
        }
    }

    return true;
}

void jstruct_base::register_field
    ( string field_type
    , string field_qualifier
    , string field_name
    , string field_name_alias
    , void*  field_address
    , void*  field_address_array_size
    , int    array_element_size
    )
{
    field_info f_info;
    size_t row = 0, col = 0;

    f_info.type_name_    = field_type;
    f_info.qualifier_    = field_qualifier;
    f_info.name_         = field_name;
    f_info.alias_        = field_name_alias;
    f_info.address_      = field_address;
    f_info.address_size_ = field_address_array_size;
    f_info.type_         = data_type(field_type, row, col);
    f_info.offset_       = array_element_size;
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

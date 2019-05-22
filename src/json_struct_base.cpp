#include "json_struct_base.h"
#include "cJSON.h"
#include <map>
#include <codecvt>
#include <typeinfo>
#include <Windows.h>
#include <algorithm>

#include <boost/lexical_cast.hpp>
#include <boost/xpressive/xpressive.hpp>

//use namespaces
using namespace boost::xpressive;

//predefined declarations
static bool is_array				(const type_info* ptype_info);
static int	array_size				(const type_info* ptype_info);
static std::pair<int,int> table_size(const type_info* ptype_info);
static bool is_bool					(const type_info* ptype_info);
static bool is_number				(const type_info* ptype_info);
static bool is_wide_string			(const type_info* ptype_info);
static bool is_narrow_string		(const type_info* ptype_info);
static bool is_wide_string_array	(const type_info* ptype_info);
static bool is_user_defined			(const type_info* ptype_info);
static void from_bool				(const type_info* field_type, void* field_address, cJSON* item);
static void from_number				(const type_info* field_type, void* field_address, cJSON* item);
static void from_bumber_array		(const type_info* field_type, void* field_address, cJSON* item, int index);


enum field_type
{
	enum_bool,
	enum_bool_array,
};

struct field_info
{
	void*				address_;
	bool				array_;
	bool				table_;
	int					array_size_;
	int					table_row_;
	int					table_col_;
	std::string			name_;
	const type_info*	type_;

	field_info()
	{
		ZeroMemory(this, sizeof(*this));
	}
};

/************************************************************************/
/* save your c++ struct size information                                */
/* key is struct name, value is struct size                             */
/************************************************************************/
static std::map<std::string, int> struct_object_size_info;

/************************************************************************/
/* array information regular expressions                                */
/************************************************************************/
//character string
static const char* wide_string			= "wchar_t\\s*\\[\\d+\\]";					//wchar_t\s*\[\d+\]
static const char* wide_string_array	= "wchar_t\\s*\\[\\d+\\]\\s*\\[\\d+\\]";	//wchar_t\s*\[\d+\]\s*\[\d+\]
static const char* narrow_string		= "char\\s*\\[\\d+\\]";						//char\s*\[\d+\]

//array size
static const char* array_len			= "\\w+\\s*\\[\\d+\\]";						//\w+\s*\[\d+\]
static const char* wchar_table_len		= "wchar_t\\s*\\[(\\d+)\\]\\s*\\[(\\d+)\\]";//wchar_t\s*\[\d+\]\s*\[\d+\]

static bool is_array(const type_info* ptype_info)
{
	return std::string::npos != std::string(ptype_info->name()).find("[");
}

static int array_size(const type_info* ptype_info)
{
	smatch sm;
	static sregex w_string_regex = sregex::compile(wide_string);

	if (regex_match(std::string(ptype_info->name()), sm, w_string_regex))
	{
		return atoi(sm[1].str().c_str());
	}

	return 0;
}

static std::pair<int, int> table_size(const type_info* ptype_info)
{
	smatch sm;
	static sregex wchar_table_regex = sregex::compile(wchar_table_len);

	std::string table_info = std::string(ptype_info->name());

	if (regex_match(table_info, sm, wchar_table_regex))
	{
		return std::make_pair(atoi(sm[1].str().c_str()), atoi(sm[2].str().c_str()));
	}

	return std::make_pair(0, 0);
}

static bool is_wide_string(const type_info* ptype_info)
{
	static sregex w_string_regex = sregex::compile(wide_string);

	return regex_match(std::string(ptype_info->name()), w_string_regex);
}

static bool is_wide_string_array(const type_info* ptype_info)
{
	static sregex w_string_a_regex = sregex::compile(wide_string_array);

	return regex_match(std::string(ptype_info->name()), w_string_a_regex);
}

static bool is_narrow_string(const type_info* ptype_info)
{
	static sregex n_string_regex = sregex::compile(narrow_string);

	return regex_match(std::string(ptype_info->name()), n_string_regex);
}

static bool is_bool(const type_info* ptype_info)
{
	return std::string::npos != std::string(ptype_info->name()).find(typeid(bool).name());
	//|| std::string::npos != std::string(ptype_info->name()).find(typeid(BOOL).name());
}

static bool is_number(const type_info* ptype_info)
{
	std::string name = std::string(ptype_info->name());

	return std::string::npos != name.find(typeid(int	).name())
		|| std::string::npos != name.find(typeid(long	).name())
		|| std::string::npos != name.find(typeid(INT64	).name())
		|| std::string::npos != name.find(typeid(DWORD	).name())
		|| std::string::npos != name.find(typeid(double	).name());
}

static bool is_user_defined(const type_info* ptype_info)
{
	return std::string::npos != std::string(ptype_info->name()).find("struct");
}

void from_bool(const type_info* field_type, void* field_address, cJSON* item)
{
	if (typeid(bool) == *field_type)
	{
		*(bool*)field_address = 1 == item->valueint ? true : false;
	}
}

void from_number(const type_info* field_type, void* field_address, cJSON* item)
{
	if (typeid(int) == *field_type)
	{
		*(int*)field_address = item->valuedouble;
	}
	else if (typeid(long) == *field_type)
	{
		*(long*)field_address = item->valuedouble;
	}
	else if (typeid(INT64) == *field_type)
	{
		*(INT64*)field_address = item->valuedouble;
	}
	else if (typeid(DWORD) == *field_type)
	{
		*(DWORD*)field_address = item->valuedouble;
	}
	else if (typeid(double) == *field_type)
	{
		*(double*)field_address = item->valuedouble;
	}
}

void from_bumber_array(const type_info* field_type, void* field_address, cJSON* item, int index)
{
	if (std::string::npos != std::string(field_type->name()).find("int"))
	{
		*((int*)field_address + index) = item->valuedouble;
	}
	else if (std::string::npos != std::string(field_type->name()).find("long"))
	{
		*((long*)field_address + index) = item->valuedouble;
	}
	else if (std::string::npos != std::string(field_type->name()).find("INT64"))
	{
		*((INT64*)field_address + index) = item->valuedouble;
	}
	else if (std::string::npos != std::string(field_type->name()).find("DWORD"))
	{
		*((DWORD*)field_address + index) = item->valuedouble;
	}
	else if (std::string::npos != std::string(field_type->name()).find("double"))
	{
		*((double*)field_address + index) = item->valuedouble;
	}
}

bool json_struct_base::from_json(std::string json)
{
	if (json.empty()) return false;

	cJSON* root = cJSON_Parse(json.c_str());

	bool success = from_json_object(root);

	cJSON_Delete(root);

	return success;
}

bool json_struct_base::from_json_object(cJSON* object)
{
	if (nullptr == object) return false;

	for (auto iter = fields_info.begin(); iter != fields_info.end(); ++iter)
	{
		field_info*			pfield_info		= *iter;

		void*				field_address	= pfield_info->address_;
		const type_info*	field_type		= pfield_info->type_;

		cJSON* item = cJSON_GetObjectItem(object, pfield_info->name_.c_str());

		if (nullptr == item								) return false;
		if (is_narrow_string(field_type)				) return false; // not support narrow character array
		if (is_bool(field_type) && is_array(field_type)	) return false;	// not support bool array

		if (is_bool(field_type))
		{
			if (cJSON_False != item->type && cJSON_True != item->type) return false;

			from_bool(field_type, field_address, item);
		}
		else if (is_number(field_type))
		{
			if (!is_array(field_type))
			{
				if (cJSON_Number != item->type) return false;

				from_number(field_type, field_address, item);
			}
			else
			{
				if (cJSON_Array != item->type) return false;

				for (int i = 0; i < cJSON_GetArraySize(item) && i < array_size(field_type); ++i)
				{
					cJSON* arrItem = cJSON_GetArrayItem(item, i);

					if (cJSON_Number != arrItem->type) return false;

					from_bumber_array(field_type, field_address, arrItem, i);
				}
			}
		}
		else if (is_wide_string(field_type))
		{
			for(int i = 0; i < array_size(field_type); ++i)
			{
				if (cJSON_String != item->type) return false;

				std::wstring ucs2 = std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t>().from_bytes(item->valuestring);

				wcsncpy_s((WCHAR*)field_address, array_size(field_type) - 1, ucs2.c_str(), ucs2.size());
			}
		}
		else if (is_wide_string_array(field_type))
		{
			if (cJSON_Array != item->type) return false;

			auto table_info = table_size(field_type);

			for (auto i = 0; i < table_info.first && i < cJSON_GetArraySize(item); ++i)
			{
				cJSON* arrItem = cJSON_GetArrayItem(item, i);

				if (cJSON_String != arrItem->type) return false;

				std::wstring ucs2 = std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t>().from_bytes(arrItem->valuestring);

				wcsncpy_s((WCHAR*)field_address + i * table_info.second, table_info.second - 1, ucs2.c_str(), ucs2.size());
			}
		}
		else if (is_user_defined(field_type))
		{
			if (!is_array(field_type))
			{
				bool success = ((json_struct_base*)field_address)->from_json_object(item);

				if (!success) return false;
			}
			else
			{
				int size = 0;
				for (auto iter = struct_object_size_info.begin(); iter != struct_object_size_info.end(); ++iter)
				{
					if (std::string::npos != std::string(field_type->name()).find(iter->first))
					{
						size = iter->second;
						break;
					}
				}
				for (int i = 0; i < array_size(field_type); ++i)
				{
					bool success = ((json_struct_base*)((BYTE*)field_address + i * size))->from_json_object(cJSON_GetArrayItem(item, i));

					if (!success) return false;
				}
			}
		}
	}

	return true;
}

void json_struct_base::register_field(std::string st_name, int st_size, const type_info* field_type, std::string field_name, void* field_address)
{
	field_info* pfield_info	= new field_info;

	pfield_info->address_	= field_address;
	pfield_info->name_		= field_name;
	pfield_info->type_		= field_type;

	fields_info.push_back(pfield_info);

	struct_object_size_info.insert(std::make_pair(st_name, st_size));
}

json_struct_base::~json_struct_base()
{
	std::for_each(fields_info.begin(), fields_info.end(), [&](field_info* pointer) { delete pointer; });
}

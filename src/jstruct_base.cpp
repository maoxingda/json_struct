#include "cJSON.h"
#include "jmacro.h"
#include "jqualifier.h"
#include "jstruct_base.h"

#include <map>
#include <codecvt>
#include <typeinfo>
#include <Windows.h>
#include <algorithm>

#include <boost/xpressive/xpressive.hpp>

using namespace boost::xpressive;

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
	type				type_;
	std::string         qualifier_;
	std::string			name_;
	std::string			alias_;
	void*				address_;
	void*				address_size_;
	int					offset_;
	int					table_row_;
	int					table_col_;
	const type_info*	field_type_;
};

struct data_type_info
{
	std::map<type, const char*> data_type_regexs;

	data_type_info()
	{
		data_type_regexs[enum_bool]					 = "bool";

		data_type_regexs[enum_number]				 = "(?:int|unsigned short|unsigned int|long|unsigned long|__int64|float|double)";
		data_type_regexs[enum_number_array]			 = "(?:int|unsigned short|unsigned int|long|unsigned long|__int64|float|double) \\[(\\d+)\\]";

		data_type_regexs[enum_wchar_array]			 = "wchar_t \\[(\\d+)\\]";
		data_type_regexs[enum_wchar_table]			 = "wchar_t \\[(\\d+)\\]\\[(\\d+)\\]";

		data_type_regexs[enum_custom]				 = "struct \\w+";
		data_type_regexs[enum_custom_array]			 = "struct \\w+ \\[(\\d+)\\]";
	}
};

/************************************************************************/
/*               data type regular expressions                          */
/************************************************************************/
static data_type_info data_type_infos;

static int array_size(const type_info* ptype_info)
{
	static cregex pattern_number_array	= cregex::compile(data_type_infos.data_type_regexs[enum_number_array]);
	static cregex pattern_wchar_array	= cregex::compile(data_type_infos.data_type_regexs[enum_wchar_array]);
	static cregex pattern_custom_array	= cregex::compile(data_type_infos.data_type_regexs[enum_custom_array]);

	cmatch array_info;

	if (regex_match(ptype_info->name(), array_info, pattern_number_array))
	{
		return atoi(array_info[1].str().c_str());
	}
	else if (regex_match(ptype_info->name(), array_info, pattern_wchar_array))
	{
		return atoi(array_info[1].str().c_str());
	}
	else if (regex_match(ptype_info->name(), array_info, pattern_custom_array))
	{
		return atoi(array_info[1].str().c_str());
	}

	return 0;
}

void table_size(const type_info* ptype_info, int& row, int& col)
{
	static cregex pattern = cregex::compile(data_type_infos.data_type_regexs[enum_wchar_table]);

	cmatch table_info;

	if (regex_match(ptype_info->name(), table_info, pattern))
	{
		row = atoi(table_info[1].str().c_str());
		col = atoi(table_info[2].str().c_str());
	}
}

static bool is_bool(const type_info* ptype_info)
{
	static cregex pattern = cregex::compile(data_type_infos.data_type_regexs[enum_bool]);

	return regex_match(ptype_info->name(), pattern);
}

static bool is_number(const type_info* ptype_info)
{
	static cregex pattern = cregex::compile(data_type_infos.data_type_regexs[enum_number]);

	return regex_match(ptype_info->name(), pattern);
}

static bool is_number_array(const type_info* ptype_info)
{
	static cregex pattern = cregex::compile(data_type_infos.data_type_regexs[enum_number_array]);

	return regex_match(ptype_info->name(), pattern);
}

static bool is_wchar_array(const type_info* ptype_info)
{
	static cregex pattern = cregex::compile(data_type_infos.data_type_regexs[enum_wchar_array]);

	return regex_match(ptype_info->name(), pattern);
}

static bool is_wchar_table(const type_info* ptype_info)
{
	static cregex pattern = cregex::compile(data_type_infos.data_type_regexs[enum_wchar_table]);

	return regex_match(ptype_info->name(), pattern);
}

static bool is_custom(const type_info* ptype_info)
{
	static cregex pattern = cregex::compile(data_type_infos.data_type_regexs[enum_custom]);

	return regex_match(ptype_info->name(), pattern);
}

static bool is_custom_array(const type_info* ptype_info)
{
	static cregex pattern = cregex::compile(data_type_infos.data_type_regexs[enum_custom_array]);

	return regex_match(ptype_info->name(), pattern);
}

static type data_type(const type_info * ptype_info, field_info * pfield_info = nullptr)
{
	if (is_bool(ptype_info))
	{
		return enum_bool;
	}
	else if (is_number(ptype_info))
	{
		return enum_number;
	}
	else if (is_number_array(ptype_info))
	{
		return enum_number_array;
	}
	else if (is_wchar_array(ptype_info))
	{
		return enum_wchar_array;
	}
	else if (is_wchar_table(ptype_info))
	{
		if (pfield_info)
		{
			table_size(ptype_info, pfield_info->table_row_, pfield_info->table_col_);
		}
		return enum_wchar_table;
	}
	else if (is_custom(ptype_info))
	{
		return enum_custom;
	}
	else if (is_custom_array(ptype_info))
	{
		return enum_custom_array;
	}

	return enum_none;
}

static void from_number(const type_info * field_type, void *field_address, cJSON * item)
{
	if (typeid(int) == *field_type)
	{
		*((int*)field_address) = item->valueint;
	}
	else if (typeid(unsigned int) == *field_type)
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
	else if (typeid(__int64) == *field_type)
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
	else if (typeid(long) == *field_type)
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
	else if (typeid(unsigned short) == *field_type)
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
	else if (typeid(unsigned long) == *field_type)
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
	else if (typeid(float) == *field_type)
	{
		*((float*)field_address) = (float)item->valuedouble;
	}
	else if (typeid(double) == *field_type)
	{
		*((double*)field_address) = item->valuedouble;
	}
}

static void from_number_array(const type_info * field_type, void *field_address, cJSON * item, int offset)
{
	if (std::string::npos != std::string(field_type->name()).find("int"))
	{
		*((int*)field_address + offset) = item->valueint;
	}
	else if (std::string::npos != std::string(field_type->name()).find("unsigned int"))
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
	else if (std::string::npos != std::string(field_type->name()).find("__int64"))
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
	else if (std::string::npos != std::string(field_type->name()).find("long"))
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
	else if (std::string::npos != std::string(field_type->name()).find("unsigned short"))
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
	else if (std::string::npos != std::string(field_type->name()).find("unsigned long"))
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
	else if (std::string::npos != std::string(field_type->name()).find("float"))
	{
		*((float*)field_address + offset) = (float)item->valuedouble;
	}
	else if (std::string::npos != std::string(field_type->name()).find("double"))
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
	if (nullptr == object)				return false;
	if (0		== fields_info.size())	return false;

	for (auto iter = fields_info.begin(); iter != fields_info.end(); ++iter)
	{
		field_info*			field_information	= (field_info*)*iter;
		void*				field_address		= field_information->address_;
		std::string			alias				= field_information->alias_;

		cJSON*				item				= nullptr;
		if (!alias.empty()) item				= cJSON_GetObjectItem((cJSON*)object, alias.c_str());
		if (!item)			item				= cJSON_GetObjectItem((cJSON*)object, field_information->name_.c_str());

		if (nullptr == item)
		{
			if (ESTR(OPTIONAL)	== field_information->qualifier_) continue;
			if (ESTR(REQUIRED)	== field_information->qualifier_) return false;
		}

		switch (field_information->type_)
		{
		case enum_bool:
			{
				if (!cJSON_IsBool(item)) return false;

				*(bool*)field_address = 1 == item->valueint ? true : false;
			}
			break;
		case enum_number:
			{
				if (!cJSON_IsNumber(item)) return false;

				from_number(field_information->field_type_, field_address, item);
			}
			break;
		case enum_number_array:
			{
				if (!cJSON_IsArray(item)) return false;

				int arrSize1 = cJSON_GetArraySize(item);
				int arrSize2 = array_size(field_information->field_type_);

				for (int i = 0; i < arrSize1 && i < arrSize2; ++i)
				{
					cJSON* arrItem = cJSON_GetArrayItem(item, i);

					if (!cJSON_IsNumber(arrItem)) return false;

					from_number_array(field_information->field_type_, field_address, arrItem, i);
				}

				*((int*)field_information->address_size_) = min(arrSize1, arrSize2);
			}
			break;
		case enum_wchar_array:
			{
				if (!cJSON_IsString(item)) return false;

				std::wstring ucs2 = std::wstring_convert < std::codecvt_utf8 < wchar_t >, wchar_t >().from_bytes(item->valuestring);

				WCHAR* dst = (WCHAR*)field_address;

				wcsncpy_s(dst, array_size(field_information->field_type_) - 1, ucs2.c_str(), ucs2.size());

				*(dst + ucs2.size()) = '\0';
			}
			break;
		case enum_wchar_table:
			{
				if (!cJSON_IsArray(item)) return false;

				int arrSize = cJSON_GetArraySize(item);

				for (auto i = 0; i < field_information->table_row_ && i < arrSize; ++i)
				{
					cJSON* arrItem = cJSON_GetArrayItem(item, i);

					if (!cJSON_IsString(arrItem)) return false;

					std::wstring ucs2 = std::wstring_convert < std::codecvt_utf8 < wchar_t >, wchar_t > ().from_bytes(arrItem->valuestring);

					WCHAR *dst = (WCHAR *) field_address + i * field_information->table_col_;

					wcsncpy_s(dst, field_information->table_col_ - 1, ucs2.c_str(), ucs2.size());

					*(dst + ucs2.size()) = '\0';
				}

				*((int*)field_information->address_size_) = min(arrSize, field_information->table_row_);
			}
			break;
		case enum_custom:
			{
                if (!cJSON_IsObject(item)) return false;

				bool success = ((jstruct_base *) field_address)->from_json_(item);

				if (!success) return false;
			}
			break;
		case enum_custom_array:
			{
                if (!cJSON_IsArray(item)) return false;

                int arrSizeReal		= cJSON_GetArraySize(item);
				int arrSizeExpected = array_size(field_information->field_type_);

				for (int i = 0; i < arrSizeExpected && i < arrSizeReal; ++i)
				{
                    cJSON* arrItem = cJSON_GetArrayItem(item, i);

                    if (!cJSON_IsObject(arrItem)) return false;

					bool success = ((jstruct_base*)((BYTE*)field_address + i * field_information->offset_))->from_json_(arrItem);

					if (!success) return false;
				}

				*((int*)field_information->address_size_) = min(arrSizeReal, arrSizeExpected);
			}
			break;
		case enum_none:
			return false;
			break;
		}
	}

	return true;
}

void jstruct_base::register_field(const type_info* field_type, std::string field_qualifier, std::string field_name, std::string field_name_alias, void* field_address, void* array_size_field_address, int offset)
{
	field_info* finfo		= new field_info;

	finfo->type_			= data_type(field_type, finfo);
	finfo->qualifier_		= field_qualifier;
	finfo->name_			= field_name;
	finfo->alias_			= field_name_alias;
	finfo->address_			= field_address;
	finfo->address_size_	= array_size_field_address;
	finfo->offset_			= offset;
	finfo->field_type_		= field_type;

	fields_info.push_back(finfo);
}

jstruct_base::~jstruct_base()
{
	std::for_each(fields_info.begin(), fields_info.end(), [&](void* pointer) { field_info* p = (field_info*)pointer; delete p; });
}

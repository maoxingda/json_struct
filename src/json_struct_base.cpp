#include "json_struct_base.h"
#include "cJSON.h"
#include <map>
#include <codecvt>
#include <typeinfo>
#include <Windows.h>


struct field_info
{
	void*				field_address;	//结构体成员字段地址
	std::string			field_name;		//结构体成员字段名称
	const type_info*	field_type;		//结构体成员字段类型

	field_info()
		: field_address(nullptr)
		, field_type(nullptr)
	{

	}
};

std::map<std::string, int> struct_info;

static bool is_bool(const type_info* ptype_info)
{
	return std::string::npos != std::string(ptype_info->name()).find(typeid(bool).name());
		//|| std::string::npos != std::string(ptype_info->name()).find(typeid(BOOL).name());
}

static bool is_number(const type_info* ptype_info)
{
	return std::string::npos != std::string(ptype_info->name()).find(typeid(int).name())
		|| std::string::npos != std::string(ptype_info->name()).find(typeid(long).name())
		|| std::string::npos != std::string(ptype_info->name()).find(typeid(INT64).name())
		|| std::string::npos != std::string(ptype_info->name()).find(typeid(DWORD).name())
		|| std::string::npos != std::string(ptype_info->name()).find(typeid(double).name());
}

static bool is_string(const type_info* ptype_info)
{
	return std::string::npos != std::string(ptype_info->name()).find(typeid(wchar_t).name());
}

static bool is_array(const type_info* ptype_info)
{
	return std::string::npos != std::string(ptype_info->name()).find("[");
}

static bool is_number_array(const type_info* ptype_info)
{
	return is_number(ptype_info) && is_array(ptype_info);
}

static bool is_user_defined(const type_info* ptype_info)
{
	return std::string::npos != std::string(ptype_info->name()).find("struct");
}

static bool is_user_defined_array(const type_info* ptype_info)
{
	return is_user_defined(ptype_info) && is_array(ptype_info);
}

static int array_size(const type_info* ptype_info)
{
	int beg_index = std::string(ptype_info->name()).find("[");
	int end_index = std::string(ptype_info->name()).find("]");

	if (std::string::npos != beg_index && std::string::npos != end_index)
	{
		return atoi(std::string(ptype_info->name()).substr(beg_index + 1, end_index - beg_index).c_str());
	}

	return 0;
}

static void from_bool(cJSON* item, const type_info* field_type, void* field_address)
{
	if (typeid(bool) == *field_type)
	{
		*(bool*)field_address = 1 == item->valueint ? true : false;
	}

}

static void from_number(cJSON* item, const type_info* field_type, void* field_address)
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

static void from_bumber_array(cJSON* item, int index, const type_info* field_type, void* field_address)
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
		field_info* pfield_info		= *iter;

		void* field_address			= pfield_info->field_address;
		const type_info* field_type	= pfield_info->field_type;

		cJSON* item = cJSON_GetObjectItem(object, pfield_info->field_name.c_str());

		if (nullptr == item) return false;

		if (!is_array(field_type))
		{
			if (is_bool(field_type))
			{
				if (cJSON_False != item->type && cJSON_True != item->type) return false;

				from_bool(item, field_type, field_address);
			}
			else if (is_number(field_type))
			{
				if (cJSON_Number != item->type) return false;

				from_number(item, field_type, field_address);
			}
			else if (is_user_defined(field_type))
			{
				bool success = ((json_struct_base*)field_address)->from_json_object(item);

				if (!success) return false;
			}
		}
		else
		{
			if (is_string(field_type))
			{
				for(int i = 0; i < array_size(field_type); ++i)
				{
					if (cJSON_String != item->type) return false;

					std::wstring ucs2 = std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t>().from_bytes(item->valuestring);

					wcsncpy_s((WCHAR*)field_address, array_size(field_type) - 1, ucs2.c_str(), ucs2.size());
				}
			}
			else if (is_number_array(field_type))
			{
				if (cJSON_Array != item->type) return false;

				for (int i = 0; i < cJSON_GetArraySize(item) && i < array_size(field_type); ++i)
				{
					cJSON* arrItem = cJSON_GetArrayItem(item, i);

					if (cJSON_Number != arrItem->type) return false;

					from_bumber_array(arrItem, i, field_type, field_address);
				}
			}
			else if (is_user_defined_array(field_type))
			{
				int size = 0;
				for (auto iter = struct_info.begin(); iter != struct_info.end(); ++iter)
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
	field_info* pfield_info		= new field_info;

	pfield_info->field_address	= field_address;
	pfield_info->field_name		= field_name;
	pfield_info->field_type		= field_type;

	fields_info.push_back(pfield_info);

	struct_info.insert(std::make_pair(st_name, st_size));
}

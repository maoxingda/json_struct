/*!
 * \brief
 *		deserialize c++ struct from json stream
 *
 * \usage
 *		1. declare struct by JSON_STRUCT(struct_name)
 *		2. register struct fields by JSON_REGISTER[_MAP]_FIELD(field_name) in constructor function
 *		3. create struct object instance and call member function from_json subsequently and that's all
 *
 * \automation
 *      1. inorder to use json2cxxstructHelper.exe tool, you must follow the rules below
 *		2. declare nested struct fields by JSON_STRUCT_FIELD[_ARRAY]
 *		3. declare struct default constructor by JSON_STRUCT_DEF_CTOR
 *
 * \note
 *		1. not support - bool				var_name([array_size])+
 *		2. not support - basic_data_type	var_name([array_size]){2,} other than wchar_t[row][col]
 *		3. not support - user_defined_type	var_name([array_size]){2,}
 *		4. can only support utf8 json stream, because the conversion from utf8 to utf16 was done internally
 *
 * \example
 *		
 *		JSON_STRUCT(date)
 *		{
 *			wchar_t year;
 *			wchar_t month;
 *			wchar_t day;
 *		
 *			date()
 *			{
 *				JSON_REGISTER_FIELD(year);
 *				JSON_REGISTER_FIELD(month);
 *				JSON_REGISTER_FIELD(day);
 *			}
 *		};
 *		
 *		JSON_STRUCT(student)
 *		{
 *			int		id;
 *			wchar_t name[32];
 *			date	birthday;
 *		
 *			student()
 *			{
 *				JSON_REGISTER_FIELD(id);
 *				JSON_REGISTER_FIELD(name);
 *				JSON_REGISTER_NESTED_FIELD(birthday);
 *			}
 *		};
 *
 *		student stu1;
 *
 *		stu1.from_json("");
 */
#pragma once
#include <list>
#include <string>


/************************************************************************/
/* the base struct where save your c++ struct fields information        */
/************************************************************************/
struct json_struct_base
{
public:
	~json_struct_base();

	bool from_json(std::string json);

private:
	bool from_json_object(void* object);

protected:
	void register_field(const type_info*, std::string, std::string, void*, int);

private:
	std::list<void*> fields_info;
};
/************************************************************************/
/*                            helper macros                             */
/************************************************************************/

#define OPTIONAL
#define REQUIRED

//declare your json struct by this macro
#define JSON_STRUCT(struct_name) struct struct_name : public json_struct_base

//declare your json struct default constructor by this macro
#define JSON_STRUCT_DEF_CTOR(struct_name) struct_name()

//declare your json struct field by this macro
#define JSON_STRUCT_FIELD(type_and_name) type_and_name

//declare your json struct field by this macro
#define JSON_STRUCT_DECL_FIELD(qualifier, type_and_name) qualifier type_and_name

//zero fill your json struct field by this macro
#define JSON_STRUCT_FIELD_FILL_ZERO(struct_name, field_name) ZeroMemory((byte*)this + offsetof(struct_name, field_name), sizeof(field_name))

//register your json struct fields by this macro in your struct constructor function
#define JSON_STRUCT_REGISTER_FIELD(qualifier, field_name) register_field(&typeid(field_name), #qualifier, #field_name, &field_name, 0)

//register your json struct fields by this macro when your struct field name is not similar to json field
#define JSON_STRUCT_REGISTER_MAP_FIELD(qualifier, field_name, map_to_name) register_field(&typeid(field_name), #qualifier, map_to_name, &field_name, 0)

//register your nested json struct field array by this macro in your struct constructor function
#define JSON_REGISTER_NESTED_FIELD(field_name) register_field(&typeid(field_name), #field_name, &field_name, sizeof(field_name[0]))

//register your nested json struct field array by this macro in your struct constructor function
#define JSON_STRUCT_REGISTER_NESTED_FIELD(qualifier, field_name) register_field(&typeid(field_name), #qualifier, #field_name, &field_name, sizeof(field_name[0]))

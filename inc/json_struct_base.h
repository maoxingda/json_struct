/*!
 * \file json_struct_base.h
 * \date 2019/05/21 12:59
 *
 * \author Administrator
 * Contact: user@company.com
 *
 * \brief
 *		deserialize c++ struct from json stream
 *
 * \usage
 *		1. declare your c++ struct by macro "JSON_STRUCT" defined in this file
 *		2. register your c++ struct fields by macro "JSON_STRUCT" in your struct constructor function
 *		3. create your c++ struct object instance and call member function from_json subsequently

 * \automation
 *      1. inorder to use json2cxxstructHelper.exe tool, you must follow the rules below
 *		2. declare your c++ nested struct fields by macro "JSON_STRUCT_FIELD[_ARRAY]?"
 *		3. declare your c++ struct default constructor by macro "JSON_STRUCT_DEF_CTOR"
 *
 * \note
 *		1. not support - bool				var_name([array_size])+
 *		2. not support - basic_data_type	var_name([array_size]){2,} other than wchar_t[row][col]
 *		3. not support - user_defined_type	var_name([array_size]){2,}
 *		4. can only support utf8 json stream, because the conversion from utf8 to utf16 was done internally
*/
#pragma once
#include <list>
#include <string>


//predefined declarations
struct	cJSON;
struct	field_info;
class	type_info;


/************************************************************************/
/* the base struct where save your c++ struct fields information        */
/************************************************************************/
struct json_struct_base
{
public:
	~json_struct_base();

	bool from_json(std::string json);

private:
	bool from_json_object(cJSON* object);

protected:
	void register_field(const type_info*, std::string, void*, int);

private:
	std::list<field_info*> fields_info;
};
/************************************************************************/
/*                            helper macros                             */
/************************************************************************/

//declare your json struct by this macro
#define JSON_STRUCT(struct_name) struct struct_name : public json_struct_base

//declare your json struct default constructor by this macro
#define JSON_STRUCT_DEF_CTOR(struct_name) struct_name()

//declare your json struct field by this macro
#define JSON_STRUCT_FIELD(type_and_name) type_and_name

//zero fill your json struct field by this macro
#define JSON_STRUCT_FIELD_FILL_ZERO(struct_name, field_name) ZeroMemory((byte*)this + offsetof(struct_name, field_name), sizeof(field_name))

//register your json struct fields by this macro in your struct constructor function
#define JSON_REGISTER_FIELD(field_name) register_field(&typeid(field_name), #field_name, &field_name, 0)

//register your json struct fields by this macro when your struct field name is not similar to json field
#define JSON_REGISTER_MAP_FIELD(field_name, json_field_name) register_field(&typeid(field_name), json_field_name, &field_name, 0)

//register your nested json struct field array by this macro in your struct constructor function
#define JSON_REGISTER_NESTED_FIELD(field_name) register_field(&typeid(field_name), #field_name, &field_name, sizeof(field_name[0]))

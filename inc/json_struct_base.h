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
 *		1. declare  your c++ struct by macro "JSON_STRUCT" defined in this file
 *		2. register your c++ struct fields by macro "JSON_STRUCT" in your struct constructor function
 *		3. create   your c++ struct object instance and call member function from_json subsequently
 *
 * \note
 *		1. not support bool array
 *		2. can only support utf8 json stream, because the conversion from utf8 to utf16 was done internally
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
	void register_field(std::string, int, const type_info*, std::string, void*);

private:
	std::list<field_info*> fields_info;
};
/************************************************************************/
/*                            helper macros                             */
/************************************************************************/

//declare your struct by this macro
#define JSON_STRUCT(struct_name) struct struct_name : public json_struct_base

//register your struct fields by this macro in your struct constructor function
#define JSON_REGISTER_FIELD(field_name) register_field(typeid(*this).name(), sizeof(*this), &typeid(field_name), #field_name, &field_name)

//register your struct fields by this macro when your struct field name is not similar to json field
#define JSON_REGISTER_MAP_FIELD(field_name, json_field_name) register_field(typeid(*this).name(), sizeof(*this), &typeid(field_name), json_field_name, &field_name)
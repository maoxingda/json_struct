#pragma once
#include <list>


//predefined declarations
struct cJSON;
struct field_info;


struct json_struct_base
{
public:
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

//register your struct fields by this macro where your struct constructor function
#define JSON_REGISTER_FIELD(field_name) register_field(typeid(*this).name(), sizeof(*this), &typeid(field_name), #field_name, &field_name)


//register your struct fields by this macro when your struct field name is not similar to json field
#define JSON_REGISTER_MAP_FIELD(field_name, json_field_name) register_field(typeid(*this).name(), sizeof(*this), &typeid(field_name), json_field_name, &field_name)
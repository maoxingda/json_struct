/*!
 * \brief
 *		deserialize c++ struct from json stream
 *
 * \usage
 *		1. declare struct by JSTRUCT(struct_name)
 *		2. register struct fields by JSON_REGISTER[_MAP]_FIELD(field_name) in constructor function
 *		3. create struct object instance and call member function from_json subsequently and that's all
 *
 * \automation
 *      1. inorder to use json2cxxstructHelper.exe tool, you must follow the rules below
 *		2. declare nested struct fields by JSTRUCT_DECL_NESTED_FIELD[_ARRAY]
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
 *		JSTRUCT(date)
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
 *		JSTRUCT(student)
 *		{
 *			int		id;
 *			wchar_t name[32];
 *			date	birthday;
 *		
 *			student()
 *			{
 *				JSON_REGISTER_FIELD(id);
 *				JSON_REGISTER_FIELD(name);
 *				JSTRUCT_REGISTER_NESTED_FIELD_ARRAY(birthday);
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
/*			the base struct where save struct fields information        */
/************************************************************************/
struct jstruct_base
{
public:
	~jstruct_base();

	bool from_json(std::string json);

private:
	bool from_json(void*);

protected:
	void register_field(const type_info*, std::string, std::string, void*, int);

private:
	std::list<void*> fields_info;
};

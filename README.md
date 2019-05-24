# deserialize c++ struct from json stream 
---
## usage

1. declare struct by ***JSON_STRUCT(struct_name)***.
2. register struct fields in constructor function by follows  
* ***JSON_STRUCT_REGISTER_FIELD(qualifier, field_name)***  
* ***JSON_STRUCT_REGISTER_MAP_FIELD(qualifier, field_name, map_to_name)***  
* ***JSON_STRUCT_REGISTER_NESTED_FIELD(qualifier, field_name)***
3. create struct object and call member function ***from_json(json_stream_utf8)***, so that's all.

## automation
1. inorder to use **json2cxxstructHelper.exe** tool, you must follow the rules below.
2. declare nested struct fields by ***JSON_STRUCT_DECL_NESTED_FIELD[_ARRAY]***.
3. declare struct default constructor.

## note
1. not support - bool var_name([array_size])+
2. not support - basic_data_type var_name([array_size]){2,} **other than** wchar_t[row][col]
3. not support - user_defined_type var_name([array_size]){2,}
4. can only support ***utf8*** json stream, because the conversion from utf8 to utf16 was done internally

###### TODO
* support serialize c++ struct to json stream  
* memory leak  

## example
```
JSON_STRUCT(date)
{
	wchar_t year;
	wchar_t month;
	wchar_t day;

	date()
	{
		JSON_REGISTER_FIELD(year);
		JSON_REGISTER_FIELD(month);
		JSON_REGISTER_FIELD(day);
	}
};

JSON_STRUCT(student)
{
	int	id;
	wchar_t name[32];
	date	birthday;

	student()
	{
		JSON_REGISTER_FIELD(id);
		JSON_REGISTER_FIELD(name);
		JSON_STRUCT_REGISTER_NESTED_FIELD(birthday);
	}
};

student stu1;

stu1.from_json("{\"id\":1001,\"name\":\"毛兴达\",\"birthday\":{\"year\":\"1990\",\"month\":\"02\",\"day\":\"16\"}}");
```

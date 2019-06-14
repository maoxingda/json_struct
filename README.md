# deserialize c++ struct from json stream
---
## usage
1. install jstructtool.exe

2. you must declare struct in form of  
```
struct struct_name
{
    REQUIRED|OPTIONAL BASIC|BASIC_ARRAY|CUSTOM|CUSTOM_ARRAY [ALIAS(alias_name)] field_type field_name;
    ....
};
```

## note
1. not support - bool var_name([array_size])+
2. not support - basic_data_type var_name([array_size]){2,} **other than** wchar_t[row][col]
3. not support - custom_type var_name([array_size]){2,}
4. can only support ***utf8*** json stream, because the conversion from utf8 to utf16 was done internally

###### TODO
* support serialize c++ struct to json stream
* output custom warning message when qualifier is not REQUIRED|OPTIONAL
* support #include

## example
```
struct date
{
	REQUIRED BASIC wchar_t year[6];
	REQUIRED BASIC wchar_t month[4];
	REQUIRED BASIC wchar_t day[4];
};

struct student
{
	REQUIRED BASIC		int	id;
	REQUIRED BASIC		wchar_t	name[32];
	REQUIRED CUSTOM		date	birthday;
	REQUIRED CUSTOM_ARRAY	date	birthday_array[2];
};

int main(int argc, char** argv)
{
	student stu1;

	assert(stu1.from_json("{\"id\":1001,\"name\":\"毛兴达\",\"birthday\":{\"year\":\"1990\",\"month\":\"02\",\"day\":\"16\"},\"birthday_array\":[{\"year\":\"1990\",\"month\":\"02\",\"day\":\"16\"},{\"year\":\"1990\",\"month\":\"02\",\"day\":\"16\"}]}"));
}
```

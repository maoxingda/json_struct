# deserialize c++ struct from json stream
---
## usage
1. install jstructtool.exe

2. add new item to microsoft visual studio vc++ project, item name pattern is ****.json.h***

3. declare struct, create struct object instance and call member function ***from_json***, so that's all

## example
```
struct date
{
    REQUIRED WCHAR_ARRAY_T wchar_t year[6];
    REQUIRED WCHAR_ARRAY_T wchar_t month[4];
    REQUIRED WCHAR_ARRAY_T wchar_t day[4];
};

struct student
{
    REQUIRED NUMBER_T ALIAS(id) int     identifier;
    REQUIRED WCHAR_ARRAY_T      wchar_t name[32];
    REQUIRED NUMBER_ARRAY_T     int     qq[2];
    REQUIRED WCHAR_TABLE_T      wchar_t email[3][32];
    REQUIRED STRUCT_T           date    birthday;
};

int main(int argc, char** argv)
{
	student stu1;

	assert(stu1.from_json("{\"identifier\":1001,\"name\":\"毛兴达\",\"qq\":[954192476],\"email\":[\"954192476@qq.com\",\"15068510522@qq.com\"],\"birthday\":{\"year\":\"1990\",\"month\":\"02\",\"day\":\"16}"));
}
```

## note
1. not support - bool var_name([array_size])+
2. not support - basic_data_type var_name([array_size]){2,} **other than** wchar_t[row][col]
3. not support - custom_type var_name([array_size]){2,}
4. can only support ***utf8*** json stream, because the conversion from utf8 to utf16 was done internally
5. gtest has memory leak

###### TODO
* serialize c++ struct to json stream
* if the optional field has a value
* null value
* derive
* concurrent
* unit test
* field type mismatch
* throw exception in constructor function

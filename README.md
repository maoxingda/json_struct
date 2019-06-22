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
    REQUIRED BASIC wchar_t year[6];
    REQUIRED BASIC wchar_t month[4];
    REQUIRED BASIC wchar_t day[4];
};

struct student
{
    REQUIRED BASIC ALIAS(id) int     identifier;
    REQUIRED BASIC           wchar_t name[32];
    REQUIRED BASIC_ARRAY     int     qq[2];
    REQUIRED BASIC_ARRAY     wchar_t email[3][32];
    REQUIRED CUSTOM          date    birthday;
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

# struct <---> json

# usage

1. install [jstructtool.exe](https://github.com/maoxingda/json_struct/releases)

2. add new item to vc++ project
[①](img/addnewitem.png)
[②](img/item.png)
[③](img/nonstdext.png)

3. declare struct, create struct object instance and call member function ***from_json <---> to_json***, so that's all

# example
```
struct date
{
    REQUIRED WCHAR_ARRAY_T wchar_t year[6];
    REQUIRED WCHAR_ARRAY_T wchar_t month[4];
    REQUIRED WCHAR_ARRAY_T wchar_t day[4];
};

struct person
{
    REQUIRED NUMBER_T ALIAS(id) int     identifier;
    REQUIRED WCHAR_ARRAY_T      wchar_t name[32];
    REQUIRED NUMBER_ARRAY_T     int     qq[2];
    REQUIRED WCHAR_TABLE_T      wchar_t email[3][32];
    REQUIRED STRUCT_T           date    birthday;
};

int main(int argc, char** argv)
{
    person p;
	
    assert(p.from_json("{\"id\":1001,\"name\":\"毛兴达\",\"qq\":[954192476],\"email\":[\"954192476@qq.com\",\"15068510522@qq.com\"],\"birthday\":{\"year\":\"1990\",\"month\":\"02\",\"day\":\"16}"));
	
    string json = p.to_json();
}
```

# support field type
1. bool
2. number, number array 
3. wchar_t array and wchar_t table
4. struct and struct array

# note
1. can only support ***utf8*** json stream, because the conversion between utf8 and utf16 was done internally
2. gtest has memory leak itself

# todo
* derive
* null value
* concurrent
* field type mismatch
* delete intermediate files
* if the optional field has a value
* get name and location field value in vs add new item wizard

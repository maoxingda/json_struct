# deserialize c++ struct from json stream 

## usage

1. declare your c++ struct by macro ***JSON_STRUCT***.
2. register your c++ struct fields by macro ***JSON_REGISTER[_MAP]?_FIELD*** in your struct constructor function.
3. create your c++ struct object instance and call member function ***from_json*** subsequently and so that's all.

## automation
1. inorder to use **json2cxxstructHelper.exe** tool, you must follow the rules below.
2. declare your c++ nested struct fields by macro ***JSON_STRUCT_FIELD[_ARRAY]?***.
3. declare your c++ struct default constructor by macro ***JSON_STRUCT_DEF_CTOR***.

## note
1. not support - bool var_name([array_size])+
2. not support - basic_data_type var_name([array_size]){2,} **other than** wchar_t[row][col]
3. not support - user_defined_type var_name([array_size]){2,}
4. can only support ***utf8*** json stream, because the conversion from utf8 to utf16 was done internally

### TODO
1. support serialize c++ sruct to json stream

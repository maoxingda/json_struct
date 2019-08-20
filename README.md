# struct <---> json

# [usage video](https://github.com/maoxingda/json_struct/tree/master/doc)

# support field type
1. jbool ---> bool
2. number and number array  
&emsp;jint    ---> int  
&emsp;juint   ---> unsigned int  
&emsp;jint64  ---> __int64  
&emsp;juint64 ---> unsigned __int64  
&emsp;jfloat  ---> float  
&emsp;jdouble ---> double  
3. string and string array (jwchar  ---> wchar_t)
4. struct and struct array (jstruct ---> struct)

# note
1. can only support ***utf8*** json stream, because the conversion between utf8 and utf16 was done internally
2. gtest has memory leak itself

# todo
* derive
* null value
* concurrent
* if the optional field has a value
* xpath
* inline comment
* use jsoncpp substitute cjson
* serialize

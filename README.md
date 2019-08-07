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
jstruct jdate
{
public jreq:
    jwchar year[6];
    jwchar month[4];
    jwchar day[4];
};

jstruct jperson
{
public jreq:
    jint     identifier;
    jwchar   name[32];
    jint     qq[2];
    jwchar   email[3][32];
    jdate    birthday;
};

int main(int argc, char** argv)
{
    jperson p;
	
    assert(p.from_json("{\"id\":1001,\"name\":\"张三\",\"qq\":[123456789,987654321],\"email\":[\"123456789@qq.com\",\"987654321@qq.com\"],\"birthday\":{\"year\":\"2000\",\"month\":\"11\",\"day\":\"16}"));
	
    string json = p.to_json();
}
```

# support field type
1. jbool ---> bool
2. number and number array
	jint    ---> int
	juint   ---> unsigned int
	jint64  ---> __int64
	juint64 ---> unsigned __int64
	jfloat  ---> float
	jdouble ---> double
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

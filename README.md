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
jstruct JDate
{
public jreq:
    jwchar year[6];
    jwchar month[4];
    jwchar day[4];
};

jstruct JPerson
{
public jreq:
    jint     identifier;
    jwchar   name[32];
    jint     qq[2];
    jwchar   email[3][32];
    JDate    birthday;
};

int main(int argc, char** argv)
{
    JPerson p;
	
    assert(p.from_json("{\"id\":1001,\"name\":\"毛兴达\",\"qq\":[954192476],\"email\":[\"954192476@qq.com\",\"15068510522@qq.com\"],\"birthday\":{\"year\":\"1990\",\"month\":\"02\",\"day\":\"16}"));
	
    string json = p.to_json();
}
```

# support field type
1. bool
2. number and number array (int, unsigned, __int64, unsigned __int64, float, double)
3. string and string array (wchar_t)
4. struct and struct array (struct)

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
* xpath
* inline comment
* use jsoncpp substitute cjson
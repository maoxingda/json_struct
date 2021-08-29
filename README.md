- [1. struct <---> json](#1-struct-----json)
- [2. usage video](#2-usage-video)
- [3. support field type](#3-support-field-type)
- [4. note](#4-note)
- [5. todo](#5-todo)

# 1. struct <---> json

# 2. [usage video](https://github.com/maoxingda/json_struct/tree/master/doc)

# 3. support field type
1. jbool ---> bool

2. number and number array  

  | jint                               | int            |
  | ---------------------------------- | -------------- |
  | juint                              | unsigned int   |
  | jint64                             | int64          |
  | juint64                            | unsigned int64 |
  | jfloat                             | float          |
  | jdouble                            | double         |

3. string and string array (jwchar  ---> wchar_t)

4. struct and struct array (jstruct ---> struct)

# 4. note
1. can only support ***utf8*** json stream, because the conversion between utf8 and utf16 was done internally
2. gtest has memory leak itself

# 5. todo
* derive
* null value
* concurrent
* if the optional field has a value
* xpath
* inline comment
* use jsoncpp substitute cjson
* serialize

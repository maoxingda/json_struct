#pragma once
#include "test/date.h"
#include <codecvt>
#include <jstruct.h>
#include <gtest/gtest.h>


/*
#pragma once


// mandatory qualifier, specify one of them
#define OPTIONAL
#define REQUIRED

// mandatory qualifier, specify one of them
#define USER_T
#define BOOL_T

#define NUMBER_T
#define NUMBER_ARRAY_T

#define WCHAR_ARRAY_T
#define WCHAR_TABLE_T

#define STRUCT_T
#define STRUCT_ARRAY_T

// optional qualifier
#define ALIAS(name)
*/

template <typename user_t>
struct test_user_field
{
    USER_T user_t user;
};

struct test_bool_field
{
    REQUIRED BOOL_T bool show;
};

template <typename number_t>
struct test_number_field
{
    REQUIRED NUMBER_T number_t number;
};

template <typename number_t>
struct test_number_array_field
{
    REQUIRED NUMBER_ARRAY_T number_t number[3];
};

struct test_wchar_array_field
{
    WCHAR_ARRAY_T REQUIRED wchar_t name[32];
};

struct test_wchar_table_field
{
    WCHAR_TABLE_T REQUIRED wchar_t email[3][32];
};

struct finger
{
    REQUIRED NUMBER_T int no;
    REQUIRED NUMBER_T int len;
};

struct person
{
    REQUIRED NUMBER_T       int     id;
    REQUIRED STRUCT_T       date    birthday;
    REQUIRED STRUCT_ARRAY_T finger  fingers[3];
};

/************************************************************************/
/* precondition:    save data
/* postcondition:   data not change
/************************************************************************/
TEST(JSTRUCT_FIELD_TYPE, USER_TYPE_NUMBER)
{
    test_user_field<int> obj;

    obj.user = 1;

    EXPECT_EQ(1, obj.user);
}

TEST(JSTRUCT_FIELD_TYPE, USER_TYPE_STRING)
{
    test_user_field<std::string> obj;

    obj.user = "qiqi";

    ASSERT_STREQ("qiqi", obj.user.c_str());
}

/************************************************************************/
/* postcondition: deserialize json stream value correctly
/************************************************************************/
TEST(JSTRUCT_FIELD_TYPE, BOOL_TYPE_TRUE)
{
    test_bool_field obj;

    obj.from_json("{\"show\":true}");

    EXPECT_EQ(true, obj.show);
}

TEST(JSTRUCT_FIELD_TYPE, BOOL_TYPE_FALSE)
{
    test_bool_field obj;

    obj.from_json("{\"show\":false}");

    EXPECT_EQ(false, obj.show);
}

TEST(JSTRUCT_FIELD_TYPE, NUMBER_TYPE_INT)
{
    test_number_field<int> obj;

    obj.from_json("{\"number\":1001}");

    EXPECT_EQ(1001, obj.number);
}

TEST(JSTRUCT_FIELD_TYPE, NUMBER_TYPE_FLOAT)
{
    test_number_field<float> obj;

    obj.from_json("{\"number\":3.14}");

    EXPECT_EQ(3.14, obj.number);
}

TEST(JSTRUCT_FIELD_TYPE, NUMBER_ARRAY_TYPE_INT)
{
    test_number_array_field<int> obj;

    obj.from_json("{\"number\":[954192476, 1506851052]}");

    EXPECT_EQ(2,            obj.number_size);
    EXPECT_EQ(954192476,    obj.number[0]);
    EXPECT_EQ(1506851052,   obj.number[1]);
}

TEST(JSTRUCT_FIELD_TYPE, WCHAR_ARRAY_TYPE)
{
    test_wchar_array_field obj;

    obj.from_json(std::wstring_convert<std::codecvt_utf8 <wchar_t>, wchar_t>().to_bytes(L"{\"name\":\"毛兴达\"}").c_str());

    EXPECT_EQ(0, wcscmp(L"毛兴达", obj.name));
}

TEST(JSTRUCT_FIELD_TYPE, WCHAR_TABLE_TYPE)
{
    test_wchar_table_field obj;

    obj.from_json("{\"email\":[\"954192476@qq.com\",\"1506851052@qq.com\"]}");

    EXPECT_EQ(2, obj.email_size);
    EXPECT_EQ(0, wcscmp(L"954192476@qq.com", obj.email[0]));
    EXPECT_EQ(0, wcscmp(L"1506851052@qq.com", obj.email[1]));
}

TEST(JSTRUCT_FIELD_TYPE, STRUCT_TYPE)
{
    person obj;

    obj.from_json("{\"id\":1001,\"birthday\":{\"year\":\"1990\",\"month\":\"02\",\"day\":\"16\"}}");

    EXPECT_EQ(1001, obj.id);
    EXPECT_EQ(0,    wcscmp(L"1990", obj.birthday.year));
    EXPECT_EQ(0,    wcscmp(L"02",   obj.birthday.month));
    EXPECT_EQ(0,    wcscmp(L"16",   obj.birthday.day));
}

TEST(JSTRUCT_FIELD_TYPE, STRUCT_ARRAY_TYPE)
{
    person obj;

    obj.from_json("{\"id\":1001,\"fingers\":[{\"no\":1,\"len\":2.5},{\"no\":2,\"len\":3.0}],\"birthday\":{\"year\":\"1990\",\"month\":\"02\",\"day\":\"16\"}}");

    EXPECT_EQ(2, obj.fingers_size);

    EXPECT_EQ(1, obj.fingers[0].no);
    EXPECT_EQ(2, obj.fingers[0].len);

    EXPECT_EQ(2, obj.fingers[1].no);
    EXPECT_EQ(3, obj.fingers[1].len);
}

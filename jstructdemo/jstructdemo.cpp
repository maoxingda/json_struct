// jstructdemo.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
//#include <vld.h>
#include <fstream>
#include <gtest/gtest.h>
#include "mjst/student.h"


#define gut

#ifdef gut
TEST(jstructdemo, int)
{
    std::fstream in("student.json");

    if (in)
    {
        std::istreambuf_iterator<char> beg(in), end;

        std::string student_json(beg, end);

        student s;

        s.sex = 1;

        EXPECT_EQ(true, s.from_json(student_json));

        EXPECT_EQ(1,    s.sex);
        EXPECT_EQ(1001, s.identifier);
        //EXPECT_EQ(1001, s.identifier);
    }
}

TEST(jstructdemo, wchar_array)
{
    std::fstream in("student.json");

    if (in)
    {
        std::istreambuf_iterator<char> beg(in), end;

        std::string student_json(beg, end);

        student s;

        s.sex = 2;

        EXPECT_EQ(true, s.from_json(student_json));

        EXPECT_EQ(2,                  s.sex);
        EXPECT_EQ(0, wcscmp(L"毛兴达", s.name));
    }
}

TEST(jstructdemo, int_array)
{
    std::fstream in("student.json");

    if (in)
    {
        std::istreambuf_iterator<char> beg(in), end;

        std::string student_json(beg, end);

        student s;

        EXPECT_EQ(true, s.from_json(student_json));

        EXPECT_EQ(2,            s.qq_size);
        EXPECT_EQ(954192476,    s.qq[0]);
        EXPECT_EQ(1506851052,   s.qq[1]);
    }
}

TEST(jstructdemo, wchar_table)
{
    std::fstream in("student.json");

    if (in)
    {
        std::istreambuf_iterator<char> beg(in), end;

        std::string student_json(beg, end);

        student s;

        EXPECT_EQ(true, s.from_json(student_json));

        EXPECT_EQ(2,                                s.email_size);
        EXPECT_EQ(0, wcscmp(L"954192476@qq.com",    s.email[0]));
        EXPECT_EQ(0, wcscmp(L"1506851052@qq.com",   s.email[1]));
    }
}

TEST(jstructdemo, struct)
{
    std::fstream in("student.json");

    if (in)
    {
        std::istreambuf_iterator<char> beg(in), end;

        std::string student_json(beg, end);

        student s;

        EXPECT_EQ(true, s.from_json(student_json));

        EXPECT_EQ(0, wcscmp(L"1990",    s.birthday.year));
        EXPECT_EQ(0, wcscmp(L"02",      s.birthday.month));
        EXPECT_EQ(0, wcscmp(L"16",      s.birthday.day));
    }
}
#endif // gut

int _tmain(int argc, _TCHAR* argv[])
{
    int result = 0;

#ifdef gut
    testing::InitGoogleTest(&argc, argv);

    result = RUN_ALL_TESTS();
#endif // gut

    std::system("pause");

	return result;
}

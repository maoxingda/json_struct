// jstructdemo.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "jperson.h"
#include <fstream>
#include <atlbase.h>


using namespace std;


int _tmain(int argc, _TCHAR* argv[])
{
    ifstream in("mjst/jperson.json");

    if (in)
    {
        istreambuf_iterator<char> beg(in), end;

        string text(beg, end);

        jperson jp1;

        jp1.from_json(text);

        wstring json = CA2T(jp1.to_json().c_str(), CP_UTF8);
    }

	return 0;
}


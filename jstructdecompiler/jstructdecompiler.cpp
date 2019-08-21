// jstructdecompiler.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "json.h"
#include <string>
#include <error.h>
#include <args.h>
#include "generator.h"
#include <boost/tokenizer.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <atlbase.h>


using namespace std;
using namespace boost;
using namespace filesystem;


int main(int argc, char* argv[])
{
    try
    {
        if (2 > argc) throw logic_error("\nno input file");

        args arg;

        arg.file_name_   = path(argv[1]).stem().string();
        arg.i_file_name_ = argv[1];
        arg.o_file_name_ = argv[1];

        arg.o_file_name_.replace(arg.o_file_name_.length() - 5, 5, ".jst");

        //cout << arg.i_file_name_ << "\n";
        //cout << arg.o_file_name_ << "\n";

        if (exists(path(arg.o_file_name_)))
        {
            remove(path(arg.o_file_name_));
        }

        std::ifstream in(arg.i_file_name_);

        if (!in) throw logic_error("\nopen file [" + arg.i_file_name_ + "] failed");

        char_separator<char> sep(";");

        istreambuf_iterator<char> beg(in), end;

        string json(beg, end);

        tokenizer< char_separator<char> > tokens(json, sep);

        for (auto iter = tokens.begin(); iter != tokens.end(); ++iter)
        {
            Json::Value root;

            if (!Json::Reader(Json::Features::strictMode()).parse(*iter, root, false))
            {
                throw logic_error(string("invalid json format:\n") + (LPCSTR)CT2A(CA2T(iter->c_str(), CP_UTF8)));
            }

            generator gen(root, arg);

            gen.write();
        }
    }
    catch (const std::exception& e)
    {
        cout << e.what() << "\n";

        return -1;
    }

    return 0;//system("pause");
}

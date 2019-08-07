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


using namespace std;
using namespace boost;
using namespace filesystem;


int main(int argc, char* argv[])
{
    if (2 > argc) return invalid_cmd_line_args;

    try
    {
        args arg;

        arg.i_file_name_ = argv[1];

        std::ifstream in(arg.i_file_name_);

        char_separator<char> sep(";");

        istreambuf_iterator<char> beg(in), end;

        string json(beg, end);

        tokenizer< char_separator<char> > tokens(json, sep);

        int num = 0;
        path ip(arg.i_file_name_);
        string dir = path(arg.i_file_name_).remove_filename().string() + "\\";
        for (auto iter = tokens.begin(); iter != tokens.end(); ++iter)
        {
            Json::Value root;

            if (!Json::Reader(Json::Features::strictMode()).parse(*iter, root, false)) continue;

            arg.o_file_name_ = (format("%1%%2%.h") % (dir + ip.stem().string()) % ++num).str();

            generator gen(root, arg);
        }
    }
    catch (const std::exception& e)
    {
        cout << e.what() << endl;
    }

    return 0;//system("pause");
}

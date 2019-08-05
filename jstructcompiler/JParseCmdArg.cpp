#include "StdAfx.h"
#include <iostream>
#include "..\inc\jmacro.h"
#include "JParseCmdArg.h"
#include <boost/filesystem.hpp>
#include <boost/tokenizer.hpp>


using namespace boost::filesystem;


JParseCmdArg::JParseCmdArg(int argc, char* argv[])
    : multi_build(false)
    , odesc_("usage")
{
    odesc_.add_options()
        ("help,h", "show this text and exit")
        ("h_out", "generate c++ header file")
        ("cpp_out", "generate c++ source file")
        ("input_file,", value(&input_file), "the input file that will be build")
        ("output_file,", value(&output_file), "generate c++ header or source file name")
        ("multi_build,m", value(&multi_build), "is it build the input file concurrently, 1 or 0")
        ("include_path,i", value(&include_path), "search include path for other jst file, multi-path separated by semicolons")
        ;

    store(parse_command_line(argc, argv, odesc_), args_);

    notify(args_);

    if (args_.count("help"))
    {
        cout << odesc_ << endl;

        throw logic_error("");
    }

    if (!input_file)  throw logic_error("the required option '--input_file' is missing");
    if (!output_file) throw logic_error("the required option '--output_file' is missing");

    if (!args_.count("h_out") && !args_.count("cpp_out"))
    {
        throw logic_error("the option '--h_out and --cpp_out' must be given at least one");
    }

    if (args_.count("h_out") && args_.count("cpp_out"))
    {
        throw logic_error("the option '--h_out and --cpp_out' must be given only one");
    }

    if (include_path)
    {
        incs_.push_back(path(*input_file).remove_filename().string());

        boost::char_separator<char>                     sep(";");
        boost::tokenizer< boost::char_separator<char> > token(*include_path, sep);

        for (auto iter = token.begin(); iter != token.end(); ++iter)
        {
            incs_.push_back(*iter);
        }
    }
}

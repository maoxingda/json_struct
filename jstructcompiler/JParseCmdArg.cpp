#include "StdAfx.h"
#include <iostream>
#include "..\inc\jmacro.h"
#include "JParseCmdArg.h"


JParseCmdArg::JParseCmdArg(int argc, char* argv[])
    : multi_build(false)
    , odesc("usage")
{
    odesc.add_options()
        ("help,h", "show this text and exit")
        ("h_out", "generate c++ header file")
        ("cpp_out", "generate c++ source file")
        ("input_file,i", value(&input_file), "the input file that will be build")
        ("output_file,o", value(&output_file), "generate c++ header or source file name")
        ("multi_build,m", value(&multi_build), "is it build the input file concurrently, 1 or 0")
        ;

    store(parse_command_line(argc, argv, odesc), args_);

    notify(args_);

    if (args_.count("help"))
    {
        cout << odesc << endl;

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
}

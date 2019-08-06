#pragma once
#include <string>
#include <boost/optional.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/options_description.hpp>


using namespace std;
using namespace boost::program_options;


class jparse_cmd_arg
{
public:
    jparse_cmd_arg(int argc, char* argv[]);

public:
    variables_map           args_;
    boost::optional<bool>   multi_build;    // concurrent build, use multi-thread
    boost::optional<string> input_file;
    boost::optional<string> output_file;
    boost::optional<string> include_path;
    vector<string>          incs_;
    options_description     odesc_;
};


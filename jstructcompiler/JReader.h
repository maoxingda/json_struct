#pragma once
#include <list>
#include <string>
#include <vector>
#include "../jstruct/jstruct/jfield_info.h"


class JParseCmdArg;

class JReader
{
public:
    JReader(std::list<std::string>& lines , std::list<struct_info>& structs , JParseCmdArg& arg);

    void read_file();
    void parse_structs();
    void parse_fields();
    void parse();
    void concurrent_parse(const std::vector<std::string>& files, std::string out_path);

public:
    JParseCmdArg&           arg_;
    std::list<std::string>& lines_;
    std::list<struct_info>& structs_;
};


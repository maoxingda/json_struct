#pragma once
#include <list>
#include <string>
#include <vector>
#include "../jstruct/jstruct/jfield_info.h"


class JParseCmdArg;

class JReader
{
    typedef std::list<std::string> slist;

public:
    JReader(slist& lines , std::list<struct_info>& structs , JParseCmdArg& arg);

    type field_type(const string& line);
    bool is_jstruct(const std::string& struct_name);
    std::string search_inc_jst(std::string file_name);
    void read_file(std::string file_name, slist& lines);
    void parse_structs();
    void parse_inc_structs(slist& lines);
    void parse_fields();
    void parse();
    void concurrent_parse(const std::vector<std::string>& files, std::string out_path);

public:
    JParseCmdArg&           arg_;
    slist&                  lines_;
    std::list<struct_info>& structs_;

private:
    slist inc_structs_;
};


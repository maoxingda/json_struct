#pragma once
#include <fstream>
#include <list>


class JParseCmdArg;
struct struct_info;

class JWriter
{
public:
    JWriter(std::list<std::string>& lines , std::list<struct_info>& structs , JParseCmdArg& arg);

    void gen_warning_code(std::ofstream& out);
    void gen_reg_fields_code(const struct_info& st_info, std::list<std::string>& reg_fields_code);
    void gen_init_fields_code(const struct_info& st_info);
    void align_reg_fields_code(std::list<std::string>& reg_fields_code);
    void write_decl_file();
    void write_impl_file();
    void save();

public:
    JParseCmdArg&           argument_;
    std::list<std::string>& lines_;
    std::list<struct_info>& structs_;
};


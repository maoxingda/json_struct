// json2cxxstructHelper.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "jmacro.h"
#include "jqualifier.h"
#include "../jstruct/jstruct/jfield_info.h"

#include <iostream>
#include <Initguid.h>

#include <boost/format.hpp>
#include <boost/foreach.hpp>
#include <boost/optional.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/xpressive/xpressive.hpp>
#include <boost/xpressive/regex_actions.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>


// {F960C1BA-33DE-485A-A6F7-5BBB3FB5C4DC}
DEFINE_GUID(VSIX_ID,
    0xf960c1ba, 0x33de, 0x485a, 0xa6, 0xf7, 0x5b, 0xbb, 0x3f, 0xb5, 0xc4, 0xdc);


using namespace boost::algorithm;
using namespace boost::xpressive;
using namespace boost::filesystem;
using namespace boost::posix_time;
namespace po = boost::program_options;
namespace pt = boost::property_tree;
using namespace po::command_line_style;


struct field_info
{
    size_t      type_   : 4;    // 15

    std::string name_;
    std::string alias_;
    std::string qualifier_;
};

struct struct_info
{
    std::string                      stname_;
    std::list<field_info>            fields_;
    std::list<std::string>           array_size_fields;
    std::list<std::string>::iterator iter_struct_beg_;
    std::list<std::string>::iterator iter_struct_end_;
};

struct version_info
{
    std::string version_;

    void load(const std::string &filename)
    {
        pt::ptree tree;

        pt::read_xml(filename, tree);

        version_ = tree.get<std::string>("compiler.version");
    }
};


static std::list<std::string>       lines;
static std::list<struct_info>       structs;
static po::variables_map            options;
static boost::optional<bool>        multi_build(false);    // concurrent build, use multi-thread
static boost::optional<std::string> input_file;
static boost::optional<std::string> output_file;
po::options_description             odesc("usage");
version_info                        verinfo;


// sub group
mark_tag alias_name(1);
mark_tag field_name(1);
mark_tag struct_name(1);
mark_tag qualifier_name(1);

// common regex expressions
static const sregex alpha_underscore    = (alpha | '_');
static const sregex identifier          = (alpha_underscore >> *(_d | alpha_underscore));
static const sregex number              = (as_xpr(ESTR(INT_T)) | ESTR(UINT_T) | ESTR(INT64_T) | ESTR(UINT64_T) | ESTR(FLOAT_T) | ESTR(DOUBLE_T));

static const sregex qualifier_col1      = (as_xpr(ESTR(REQUIRED)) | ESTR(OPTIONAL));
static const sregex qualifier_col2      = (as_xpr(ESTR(BOOL_T)) | number | ESTR(WCHAR_T) | ESTR(STRUCT_T));
static const sregex qualifier_col3      = (as_xpr(ESTR(ALIAS)) >> '(' >> (alias_name = identifier) >> ')');

static const sregex qualifier           = (qualifier_col1 | qualifier_col2 | qualifier_col3);

static const sregex field               = (bos >> *_s >> repeat<2, 3>(qualifier >> +_s) >> !(identifier >> +_s) >> (field_name = identifier) >> repeat<0, 2>(as_xpr("[") >> (+_d | identifier) >> "]") >> *_s >> ';');

static const sregex user_field          = (bos >> *_s >> (s1 = ESTR(USER_T) >> +_s) >> identifier);
static const sregex bool_field          = (bos >> *_s >> repeat<2, 3>((as_xpr(ESTR(REQUIRED)) | as_xpr(ESTR(BOOL_T)) | qualifier_col3) >> +_s) >> (field_name = identifier) >> *_s >> ';');

static const sregex number_field        = (bos >> *_s >> repeat<2, 3>((as_xpr(ESTR(REQUIRED)) | number | qualifier_col3) >> +_s) >> (field_name = identifier) >> *_s >> ';');
static const sregex number_array_field  = (bos >> *_s >> repeat<2, 3>((as_xpr(ESTR(REQUIRED)) | number | qualifier_col3) >> +_s) >> (field_name = identifier) >> repeat<1, 1>(as_xpr("[") >> (+_d | identifier) >> "]") >> *_s >> ';');

static const sregex wchar_array_field   = (bos >> *_s >> repeat<2, 3>((as_xpr(ESTR(REQUIRED)) | ESTR(WCHAR_T) | qualifier_col3) >> +_s) >> (field_name = identifier) >> repeat<1, 1>(as_xpr("[") >> (+_d | identifier) >> "]") >> *_s >> ';');
static const sregex wchar_table_field   = (bos >> *_s >> repeat<2, 3>((as_xpr(ESTR(REQUIRED)) | ESTR(WCHAR_T) | qualifier_col3) >> +_s) >> (field_name = identifier) >> repeat<2, 2>(as_xpr("[") >> (+_d | identifier) >> "]") >> *_s >> ';');

static const sregex struct_field        = (bos >> *_s >> repeat<2, 3>((as_xpr(ESTR(REQUIRED)) | ESTR(STRUCT_T) | qualifier_col3) >> +_s) >> identifier >> +_s >> (field_name = identifier) >> *_s >> ';');
static const sregex struct_array_field  = (bos >> *_s >> repeat<2, 3>((as_xpr(ESTR(REQUIRED)) | ESTR(STRUCT_T) | qualifier_col3) >> +_s) >> identifier >> +_s >> (field_name = identifier) >> repeat<1, 1>(as_xpr("[") >> (+_d | identifier) >> "]") >> *_s >> ';');

static const sregex other_jst           = bos >> (s1 = "#include" >> +_s >> '"' >> identifier) >> ".jst" >> before('"' >> *_s >> eos);

static bool is_user_field(std::string& line)
{
    return regex_search(line, user_field);
    //smatch sm;

    //if (regex_search(line, sm, user_field))
    //{
    //    replace_first(line, sm[s1], "");

    //    return true;
    //}

    //return false;
}

static bool is_field(std::string& line)
{
    return regex_search(line, field);
}

static type field_type(const string& line)
{
    if (regex_search(line, bool_field))
    {
        return enum_bool;
    }
    else if (regex_search(line, number_field))
    {
        return enum_number;
    }
    else if (regex_search(line, number_array_field))
    {
        return enum_number_array;
    }
    else if (regex_search(line, wchar_array_field))
    {
        return enum_wchar_array;
    }
    else if (regex_search(line, wchar_table_field))
    {
        return enum_wchar_table;
    }
    else if (regex_search(line, struct_field))
    {
        return enum_struct;
    }
    else if (regex_search(line, struct_array_field))
    {
        return enum_struct_array;
    }

    return enum_none;
}

static std::string field_qualifier(const std::string& line)
{
    smatch sm;

    std::string qualifier;

    static const sregex re_qualifier = (qualifier_name = repeat<2, 3>(::qualifier >> +_s));

    regex_search(line, sm, re_qualifier) ? qualifier += sm[qualifier_name] : 0;

    return qualifier;
}

static std::string qualifier_required(const std::string& full_qualifier)
{
    smatch sm;
    std::string ret;

    static const sregex re_qualifier_col1 = (s1 = qualifier_col1);

    // if missing
    if (regex_search(full_qualifier, sm, re_qualifier_col1))
    {
        ret = sm[s1];

        // if repeated
        if (regex_search(full_qualifier.substr(sm[s1].second - full_qualifier.begin()), sm, re_qualifier_col1))
        {
            ret = "";
        }
    }

    return ret;
}

static std::string qualifier_type(const std::string& full_qualifier)
{
    smatch sm;
    std::string ret;

    static const sregex re_qualifier_col2 = (s1 = qualifier_col2);

    // if missing
    if (regex_search(full_qualifier, sm, re_qualifier_col2))
    {
        ret = sm[s1];

        if (regex_search(full_qualifier.substr(sm[s1].second - full_qualifier.begin()), sm, re_qualifier_col2))
        {
            ret = "";
        }
    }

    return ret;
}

static std::string qualifier_alias(const std::string& line)
{
    smatch sm;

    if (regex_search(line, sm, qualifier_col3))
    {
        return sm[alias_name];
    }

    return "";
}

static std::string parse_field_name(const std::string& line)
{
    smatch sm;

    if (regex_search(line, sm, field))
    {
        return sm[field_name];
    }

    return "";
}

static void remove_qualifiers(std::string& line)
{
    static const sregex qualifier_space = repeat<2, 3>(qualifier >> +_s);

    line = regex_replace(line, qualifier_space, "");
}

static int is_single_line_comment(std::string line)
{
    static const sregex re1 = bos >> *_s >> "//" >> *_ >> eos;
    static const sregex re2 = bos >> *_s >> "/*" >> *_ >> "*/" >> eos;

    if (regex_search(line, re1)) return 1;
    if (regex_search(line, re2)) return 2;

    return 0;
}

static bool is_multiline_comment_beg(std::string line)
{
    static const sregex re = bos >> *_s >> "/*";

    return regex_search(line, re);
}

static bool is_multiline_comment_end(std::string line)
{
    static const sregex re = ((bos >> *_s >> "*/") | (*_ >> "*/" >> *_s)) >> eos;

    return regex_search(line, re);
}

static bool is_in_comment(const std::string& line)
{
    static bool comment = false;

    int single_line_comment = is_single_line_comment(line);

    if (1 == single_line_comment) return true;
    if (2 == single_line_comment)
    {
        comment = false;

        return true;
    }

    if (is_multiline_comment_beg(line))
    {
        comment = true;

        return true;
    }
    else if (is_multiline_comment_end(line))
    {
        comment = false;

        return true;
    }
    else if (comment)
    {
        return true;
    }

    return false;
}

static void align(std::string& line1, std::string& line2, const sregex& re1, const sregex& re2, unsigned recursive_call_depth)
{
    if (64 < recursive_call_depth) return; // avoid stack overflow

    smatch sm1;
    smatch sm2;

    if (regex_search(line1, sm1, re1) && regex_search(line2, sm2, re2))
    {
        auto offset1 = sm1[s1].second - line1.begin();
        auto offset2 = sm2[s1].second - line2.begin();

        if (offset1 != offset2)
        {
            line1.insert(sm1[s1].first - line1.begin(), " ");

            align(line1, line2, re1, re2, recursive_call_depth + 1);
        }
    }
}

static std::string file_name(const std::string& file_name)
{
    return path(file_name).filename().string();
}

static void read_file(const std::string& file_name)
{
    std::ifstream in(file_name);

    if (in)
    {
        std::string line;

        while (getline(in, line))
        {
            lines.push_back(line);
        }

        in.close();
    }
    else
    {
        throw std::logic_error("open '" + file_name + "' failed");
    }
}

static void parse_structs(const std::string& file_name)
{
    smatch              sm;
    struct_info         st_info;
    std::string         jst_base = " : public jstruct_base";
    static const sregex re_struct_end = bos >> *_s >> '}' >> *_s >> ';';
    static const sregex re_struct_beg = as_xpr("struct") >> +_s >> (struct_name = identifier);

    for (auto iter = lines.begin(); iter != lines.end(); ++iter)
    {
        if (iter->empty() || is_in_comment(*iter)) continue; // skip empty line and comment

        if (regex_search(*iter, sm, re_struct_beg))
        {
            st_info.stname_             = sm[struct_name];
            st_info.iter_struct_beg_    = iter;

            iter->insert(sm[struct_name].second, jst_base.begin(), jst_base.end());
        }
        else if (regex_search(*iter, re_struct_end))
        {
            st_info.iter_struct_end_ = iter;

            structs.push_back(st_info);
        }
    }

    if (!structs.size()) throw std::logic_error("not find struct in '" + file_name + "'");
}

size_t single_line_comment_offset(const std::string& line)
{
    size_t idx1 = line.find("//");
    size_t idx2 = line.find("/*");

    if (std::string::npos != idx1 && std::string::npos != idx2)
    {
        return std::min(idx1, idx2);
    }
    else if (std::string::npos == idx1 && std::string::npos == idx2)
    {
        return 0;
    }
    else
    {
        return std::min(idx1, idx2);
    }
}

static void parse_fields()
{
    for (auto iter1 = structs.begin(); iter1 != structs.end(); ++iter1)
    {
        for (auto iter2 = iter1->iter_struct_beg_; iter2 != iter1->iter_struct_end_; ++iter2)
        {
            auto& line = *iter2;

            if (line.empty())           continue;   // skip empty line
            if (is_in_comment(line))    continue;   // skip comment
            if (is_user_field(line))    continue;   // skip user field
            if (!is_field(line))        continue;   // skip not field line

            field_info f_info;

            f_info.type_ = field_type(line);

            if (enum_none == f_info.type_)
            {
                lines.insert(iter2, "    #error unknow field");

                continue;
            }

            f_info.name_      = parse_field_name(line);
            f_info.alias_     = qualifier_alias(line);
            f_info.qualifier_ = field_qualifier(line);

            iter1->fields_.push_back(f_info);

            //remove_qualifiers(line);

            // define array size variable
            if (enum_number_array == f_info.type_ || enum_wchar_table == f_info.type_ || enum_struct_array == f_info.type_)
            {
                auto iter3 = lines.insert(++iter2, (boost::format("    int %1%_size;") % f_info.name_).str());

                --iter2;

                iter1->array_size_fields.push_back((boost::format("%1%_size") % f_info.name_).str());

                sregex re = sregex::compile(f_info.name_);

                align(*iter3, line, (s1 = re) >> before("_size"), (s1 = re), 1);
            }
        }
    }
}

static void gen_warning_code(std::ofstream& out)
{
    out << "/****************************************************************************" << "\n";
    out << "** register struct field code from reading C++ file '" << file_name(*input_file) << "'\n";
    out << "**" << "\n";
    out << "** created: " << to_simple_string(second_clock::local_time()) << "\n";
    out << "**      by: the json struct compiler version " << verinfo.version_ << "\n";
    out << "**" << "\n";
    out << "** warning! all changes made in this file will be lost!" << "\n";
    out << "*****************************************************************************/" << "\n";
}

static void gen_reg_fields_code(const struct_info& st_info, std::list<std::string>& reg_fields_code)
{
    for (auto iter = st_info.fields_.begin(); iter != st_info.fields_.end(); ++iter)
    {
        if (std::string::npos != iter->qualifier_.find(ESTR(ALIAS)))
        {
            switch (iter->type_)
            {
            case enum_bool:
                {
                    reg_fields_code.push_back((boost::format("        %1%(%2%, %3%, %4%);") % ESTR(JSTRUCT_REG_BOOL_FIELD_ALIAS) % qualifier_required(iter->qualifier_) % iter->name_ % iter->alias_).str());
                }
                break;

            case enum_number:
                {
                    reg_fields_code.push_back((boost::format("        %1%(%2%, %3%, %4%);") % ESTR(JSTRUCT_REG_NUMBER_FIELD_ALIAS) % qualifier_required(iter->qualifier_) % iter->name_ % iter->alias_).str());
                }
                break;
            case enum_number_array:
                {
                    reg_fields_code.push_back((boost::format("        %1%(%2%, %3%, %4%);") % ESTR(JSTRUCT_REG_NUMBER_ARRAY_FIELD_ALIAS) % qualifier_required(iter->qualifier_) % iter->name_ % iter->alias_).str());
                }
                break;

            case enum_wchar_array:
                {
                    reg_fields_code.push_back((boost::format("        %1%(%2%, %3%, %4%);") % ESTR(JSTRUCT_REG_WCHAR_ARRAY_FIELD_ALIAS) % qualifier_required(iter->qualifier_) % iter->name_ % iter->alias_).str());
                }
                break;
            case enum_wchar_table:
                {
                    reg_fields_code.push_back((boost::format("        %1%(%2%, %3%, %4%);") % ESTR(JSTRUCT_REG_WCHAR_TABLE_FIELD_ALIAS) % qualifier_required(iter->qualifier_) % iter->name_ % iter->alias_).str());
                }
                break;

            case enum_struct:
                {
                    reg_fields_code.push_back((boost::format("        %1%(%2%, %3%, %4%);") % ESTR(JSTRUCT_REG_STRUCT_FIELD_ALIAS) % qualifier_required(iter->qualifier_) % iter->name_ % iter->alias_).str());
                }
                break;
            case enum_struct_array:
                {
                    reg_fields_code.push_back((boost::format("        %1%(%2%, %3%, %4%);") % ESTR(JSTRUCT_REG_STRUCT_ARRAY_FIELD_ALIAS) % qualifier_required(iter->qualifier_) % iter->name_ % iter->alias_).str());
                }
                break;
            }
        }
        else
        {
            switch (iter->type_)
            {
            case enum_bool:
                {
                    reg_fields_code.push_back((boost::format("        %1%(%2%, %3%);") % ESTR(JSTRUCT_REG_BOOL_FIELD) % qualifier_required(iter->qualifier_) % iter->name_).str());
                }
                break;

            case enum_number:
                {
                    reg_fields_code.push_back((boost::format("        %1%(%2%, %3%);") % ESTR(JSTRUCT_REG_NUMBER_FIELD) % qualifier_required(iter->qualifier_) % iter->name_).str());
                }
                break;
            case enum_number_array:
                {
                    reg_fields_code.push_back((boost::format("        %1%(%2%, %3%);") % ESTR(JSTRUCT_REG_NUMBER_ARRAY_FIELD) % qualifier_required(iter->qualifier_) % iter->name_).str());
                }
                break;

            case enum_wchar_array:
                {
                reg_fields_code.push_back((boost::format("        %1%(%2%, %3%);") % ESTR(JSTRUCT_REG_WCHAR_ARRAY_FIELD) % qualifier_required(iter->qualifier_) % iter->name_).str());
                }
                break;
            case enum_wchar_table:
                {
                    reg_fields_code.push_back((boost::format("        %1%(%2%, %3%);") % ESTR(JSTRUCT_REG_WCHAR_TABLE_FIELD) % qualifier_required(iter->qualifier_) % iter->name_).str());
                }
                break;

            case enum_struct:
                {
                    reg_fields_code.push_back((boost::format("        %1%(%2%, %3%);") % ESTR(JSTRUCT_REG_STRUCT_FIELD) % qualifier_required(iter->qualifier_) % iter->name_).str());
                }
                break;
            case enum_struct_array:
                {
                    reg_fields_code.push_back((boost::format("        %1%(%2%, %3%);") % ESTR(JSTRUCT_REG_STRUCT_ARRAY_FIELD) % qualifier_required(iter->qualifier_) % iter->name_).str());
                }
                break;
            }
        }
    }
}

static void align_reg_fields_code(std::list<std::string>& reg_fields_code)
{
    smatch sm;
    int max_qualifier_index = 0;
    std::list<std::string>::iterator max_qualifier_iter;
    static const sregex re_qualifier_col1 = (s1 = qualifier_col1);
    for (auto iter2 = reg_fields_code.begin(); iter2 != reg_fields_code.end(); ++iter2)
    {
        if (!regex_search(*iter2, sm, re_qualifier_col1))
        {
            static boost::format fmt("#error \"missing or error qualifier, only support %1% and %2%\";");

            fmt % ESTR(REQUIRED) % ESTR(OPTIONAL);

            iter2->insert(0, fmt.str().c_str());

            continue;
        }

        auto offset = sm[s1].first - iter2->begin();

        if (max_qualifier_index < offset)
        {
            max_qualifier_index = offset;
            max_qualifier_iter  = iter2;
        }
    }
    for (auto iter2 = reg_fields_code.begin(); iter2 != reg_fields_code.end(); ++iter2)
    {
        if (iter2 != max_qualifier_iter)
        {
            align(*iter2, *max_qualifier_iter, re_qualifier_col1, re_qualifier_col1, 1);
        }
    }
}

static void gen_init_fields_code(const struct_info& st_info)
{
    for (auto iter2 = st_info.fields_.begin(); iter2 != st_info.fields_.end(); ++iter2)
    {
        if (-1 == iter2->qualifier_.find(ESTR(STRUCT_T)))
        {
            lines.insert(st_info.iter_struct_end_, (boost::format("        JSTRUCT_INIT_FIELD_ZERO(%1%, %2%);") % st_info.stname_ % iter2->name_).str());
        }
    }

    for (auto iter2 = st_info.array_size_fields.begin(); iter2 != st_info.array_size_fields.end(); ++iter2)
    {
        lines.insert(st_info.iter_struct_end_, (boost::format("        JSTRUCT_INIT_FIELD_ZERO(%1%, %2%);") % st_info.stname_ % *iter2).str());
    }
}

static void write_decl_file(const std::string& o_file_name)
{
    std::ofstream out(o_file_name);

    for (auto iter1 = structs.begin(); iter1 != structs.end(); ++iter1)
    {
        auto& st_info  = *iter1;
        auto& position = iter1->iter_struct_end_;

        // generate register field code in construct function
        lines.insert(position, (boost::format("\n    %1%()") % st_info.stname_).str());
        lines.insert(position, "    {");
        {
            std::list<std::string> reg_fields_code;

            gen_reg_fields_code(st_info, reg_fields_code);

            align_reg_fields_code(reg_fields_code);

            BOOST_FOREACH(auto item, reg_fields_code) lines.insert(position, item);

            // insert empty line
            lines.insert(position, "");

            // field initialization
            gen_init_fields_code(st_info);
        }
        lines.insert(position, "    }");
    }

    // save
    gen_warning_code(out);

    for (auto iter = lines.begin(); iter != lines.end(); ++iter)
    {
        //replace_first(*iter, ".jst", ".h");

        *iter = regex_replace(*iter, other_jst, s1 + ".h"); // execute more quikly

        out << *iter << "\n";
    }
    out.close();

    lines.clear();
    structs.clear();
}

static int write_impl_file(const std::string& o_file_name)
{
    throw std::logic_error(std::string(__FUNCTION__) + " not implemented!");

    lines.clear();
    structs.clear();
}

static bool is_out_of_date(const std::string& i_file_name, const std::string& o_file_name)
{
    path pif(i_file_name), pof(o_file_name);

    return !exists(pof) || last_write_time(pif) > last_write_time(pof);
}

static void parse(std::string i_file_name, std::string o_file_name)
{
    read_file(i_file_name);

    parse_structs(i_file_name);

    parse_fields();

    if (options.count("h_out"))
    {
        write_decl_file(o_file_name);
    }
    else if (options.count("cpp_out"))
    {
        write_impl_file(o_file_name);
    }
}

static void concurrent_parse(const std::vector<std::string>& files, std::string out_path, std::string file_ext)
{
    throw std::logic_error(std::string(__FUNCTION__) + " not implemented!");
}

static std::string cosn(const char* option_long_name, char character)
{
    std::string option_name(option_long_name);

    option_name += ",";
    option_name += character;

    return option_name;
}

void read_command_line_argument(int argc, char* argv[])
{
    std::string onif = cosn(ESTR(input_file), 'i');
    std::string onof = cosn(ESTR(output_file), 'o');
    std::string onmb = cosn(ESTR(multi_build), 'm');

    odesc.add_options()
        ("help,h",                               "show this text and exit")
        ("h_out",                                "generate c++ header file")
        ("cpp_out",                              "generate c++ source file")
        (onif.c_str(), po::value(&input_file),   "the input file that will be build")
        (onof.c_str(), po::value(&output_file),  "generate c++ header or source file name")
        (onmb.c_str(), po::value(&multi_build),  "is it build the input file concurrently, 1 or 0")
        ;

    store(po::parse_command_line(argc, argv, odesc), options);

    notify(options);
}

#include <shlobj.h>
#pragma comment(lib, "shell32.lib")

int main(int argc, char *argv[])
{
    try
    {
        read_command_line_argument(argc, argv);

        if (options.count("help"))
        {
            std::cout << odesc << std::endl;

            return 0;
        }

        if (!input_file)  throw std::logic_error("the required option '--input_file' is missing");
        if (!output_file) throw std::logic_error("the required option '--output_file' is missing");

        if (!options.count("h_out") && !options.count("cpp_out"))
        {
            throw std::logic_error("the option '--h_out and --cpp_out' must be given at least one");
        }

        if (options.count("h_out") && options.count("cpp_out"))
        {
            throw std::logic_error("the option '--h_out and --cpp_out' must be given only one");
        }

        char my_documents[MAX_PATH] = { 0 };

        SHGetFolderPathA(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, my_documents);

        verinfo.load(my_documents + std::string("\\Visual Studio 2010\\Addins\\version.xml"));

        parse(*input_file, *output_file);

        return 0;
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << "\n" << std::endl;

        return -1;
    }
}
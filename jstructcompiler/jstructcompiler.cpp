// json2cxxstructHelper.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "jmacro.h"
#include "jqualifier.h"
#include <iostream>
#include <Initguid.h>
#include <boost/format.hpp>
#include <boost/foreach.hpp>
#include <boost/optional.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/xpressive/xpressive.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#define compiler_version "V1.0.0"

// {F960C1BA-33DE-485A-A6F7-5BBB3FB5C4DC}
DEFINE_GUID(VSIX_ID,
    0xf960c1ba, 0x33de, 0x485a, 0xa6, 0xf7, 0x5b, 0xbb, 0x3f, 0xb5, 0xc4, 0xdc);

using namespace boost::algorithm;
using namespace boost::xpressive;
using namespace boost::filesystem;
using namespace boost::posix_time;
namespace po = boost::program_options;
using namespace po::command_line_style;


struct field_info
{
    std::string name_;
    std::string alias_;
    std::string qualifier_;
    bool        qualifier_error_;
};

struct struct_info
{
    std::string                      stname_;
    std::list<field_info>            fields_;
    std::list<std::string>           array_size_fields;
    std::list<std::string>::iterator iter_struct_beg_;
    std::list<std::string>::iterator iter_struct_end_;
};

static std::list<std::string>       lines;
static std::list<struct_info>       structs;
static std::map<int, const char*>*  perror_msg = nullptr;
static po::variables_map            options;
static boost::optional<bool>        multi_build(false);    // concurrent build, use multi-thread
static boost::optional<bool>        always_build(false);   // ignore file last write time
static std::vector< std::string >   input_files;
static boost::optional<std::string> output_path;
po::options_description             odesc("usage");
po::positional_options_description  podesc;

static std::string error_msg(std::string msg_id)
{
    //msg_id[0] -= 32;

    replace_all(msg_id, "_", " ");

    return msg_id;
}

static bool is_user_field(std::string& line)
{
    static smatch sm;
    static sregex re = sregex::compile((boost::format("^\\s+(\\b%1%\\b\\s+)\\w+") % ESTR(USER)).str());

    if (regex_search(line, sm, re))
    {
        replace_first(line, sm[1], "");

        return true;
    }

    return false;
}

static bool is_field(std::string& line)
{
    boost::format fmt("^\\s*(?:(?:(?:%1%|%2%|%3%|%4%|%5%|%6%|%7%\\(\\s*[a-zA-Z_$][a-zA-Z0-9_$]*\\s*\\))\\s+){2,3})(?:[a-zA-Z_$][a-zA-Z0-9_$]*\\s+){1,2}[a-zA-Z_$][a-zA-Z0-9_$]*(?:\\[\\w+\\]){0,2}\\s*;");

    fmt % ESTR(REQUIRED) % ESTR(OPTIONAL) % ESTR(BASIC) % ESTR(BASIC_ARRAY) % ESTR(CUSTOM) % ESTR(CUSTOM_ARRAY) % ESTR(ALIAS);

    static sregex re = sregex::compile(fmt.str());

    return regex_search(line, re);
}

static std::string field_qualifier(const std::string& line)
{
    smatch      sm1, sm2, sm3;
    std::string qualifier;

    static sregex re1   = sregex::compile((boost::format("(%1%|%2%)\\s+\\w+") % ESTR(REQUIRED) % ESTR(OPTIONAL)).str());
    static sregex re2   = sregex::compile((boost::format("(%1%|%2%|%3%|%4%)\\s+\\w+") % ESTR(BASIC_ARRAY) % ESTR(CUSTOM_ARRAY) % ESTR(BASIC) % ESTR(CUSTOM)).str());
    static sregex re3   = sregex::compile((boost::format("(%1%)\\(\\s*[a-zA-Z_$][a-zA-Z0-9_$]*\\s*\\)\\s+\\w+") % ESTR(ALIAS)).str());

    if (regex_search(line, sm1, re1))
    {
        qualifier += sm1[1];
    }
    if (regex_search(line, sm2, re2))
    {
        qualifier += sm2[1];
    }
    if (regex_search(line, sm3, re3))
    {
        qualifier += sm3[1];
    }

    return qualifier;
}

static std::string qualifier_required(const std::string& full_qualifier)
{
    std::string     ret;
    static smatch   sm;

    static sregex re = sregex::compile((boost::format("(%1%|%2%)") % ESTR(REQUIRED) % ESTR(OPTIONAL)).str());

    // if missing
    if (regex_search(full_qualifier, sm, re))
    {
        ret = sm[1];

        // if repeated
        if (regex_search(full_qualifier.substr(sm[1].second - full_qualifier.begin()), sm, re))
        {
            ret = "";
        }
    }

    return ret;
}

static std::string qualifier_type(const std::string& full_qualifier)
{
    std::string     ret;
    static smatch   sm;

    static sregex re = sregex::compile((boost::format("(%1%|%2%|%3%|%4%)") % ESTR(BASIC_ARRAY) % ESTR(CUSTOM_ARRAY) % ESTR(BASIC) % ESTR(CUSTOM)).str());

    // if missing
    if (regex_search(full_qualifier, sm, re))
    {
        ret = sm[1];

        // if repeated
        if (regex_search(full_qualifier.substr(sm[1].second - full_qualifier.begin()), sm, re))
        {
            ret = "";
        }
    }

    return ret;
}

static std::string qualifier_alias(const std::string& line)
{
    static smatch sm;

    static sregex re = sregex::compile((boost::format("%1%\\(\\s*([a-zA-Z_$][a-zA-Z0-9_$]*)\\s*\\)\\s+") % ESTR(ALIAS)).str());

    if (regex_search(line, sm, re))
    {
        return sm[1];
    }

    return "";
}

static std::string field_name(const std::string& line)
{
    smatch sm;

    static sregex re = sregex::compile("([a-zA-Z_$][a-zA-Z0-9_$]*)(?:\\[\\w+\\]){0,2}\\s*;");

    if (regex_search(line, sm, re))
    {
        return sm[1];
    }

    return "";
}

static std::string struct_name(const std::string& declaration)
{
    smatch sm;

    if (regex_search(declaration, sm, sregex::compile("struct\\s+([a-zA-Z_$][a-zA-Z0-9_$]*)")))
    {
        return sm[1];
    }

    return "";
}

static void remove_qualifiers(std::string& line)
{
    smatch sm;

    boost::format fmt("(((%1%|%2%|%3%|%4%|%5%|%6%|%7%\\(\\s*\\w+\\s*\\))\\s+){2,3})");

    fmt % ESTR(REQUIRED) % ESTR(OPTIONAL) % ESTR(BASIC) % ESTR(BASIC_ARRAY) % ESTR(CUSTOM) % ESTR(CUSTOM_ARRAY) % ESTR(ALIAS);

    static sregex re = sregex::compile(fmt.str());

    if (regex_search(line, sm, re))
    {
        line.replace(sm[1].first, sm[1].second, "");
    }
}

static bool is_single_line_comment(std::string line)
{
    static sregex single_line_comment_re = sregex::compile("^\\s*//");

    return regex_search(line, single_line_comment_re);
}

static bool is_multiline_comment_beg(std::string line)
{
    static sregex multiline_comment_beg_re = sregex::compile("^\\s*/\\*");

    return regex_search(line, multiline_comment_beg_re);
}

static bool is_multiline_comment_end(std::string line)
{
    static sregex multiline_comment_end_re = sregex::compile("^\\s*\\*/");

    return regex_search(line, multiline_comment_end_re);
}

static bool is_in_comment(const std::string& line)
{
    static bool comment = false;

    if (is_single_line_comment(line)) return true;

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

static void align(std::string& line1, std::string& line2, const sregex& re)
{
    smatch sm1;
    smatch sm2;

    if (regex_search(line1, sm1, re) && regex_search(line2, sm2, re))
    {
        auto offset1 = sm1[1].first - line1.begin();
        auto offset2 = sm2[1].first - line2.begin();

        if (offset1 != offset2)
        {
            line1.insert(sm1[1].first - line1.begin(), " ");

            align(line1, line2, re);
        }
    }
}

static std::string file_base_name(const std::string& file_name)
{
    return path(file_name).stem().string();
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

static void read_struct(const std::string& file_name)
{
    smatch              sm;
    struct_info         st_info;
    std::string         jst_base = " : public jstruct_base";

    sregex struct_end_re = sregex::compile("^\\s*\\}\\s*;");
    sregex struct_beg_re = sregex::compile("struct\\s+([a-zA-Z_$][a-zA-Z0-9_$]*)");

    for (auto iter = lines.begin(); iter != lines.end(); ++iter)
    {
        if (iter->empty() || is_in_comment(*iter)) continue; // skip empty line and comment

        if (regex_search(*iter, sm, struct_beg_re))
        {
            st_info.iter_struct_beg_ = iter;

            iter->insert(sm[1].second, jst_base.begin(), jst_base.end());
        }
        else if (regex_search(*iter, struct_end_re))
        {
            st_info.iter_struct_end_ = iter;

            structs.push_back(st_info);
        }
    }

    if (!structs.size()) throw std::logic_error("not find struct in '" + file_name + "'");
}

static void read_fields()
{
    for (auto iter1 = structs.begin(); iter1 != structs.end(); ++iter1)
    {
        iter1->stname_ = struct_name(*iter1->iter_struct_beg_);

        for (auto iter2 = iter1->iter_struct_beg_; iter2 != iter1->iter_struct_end_; ++iter2)
        {
            auto& line = *iter2;

            if (line.empty())           continue;   // skip empty line
            if (is_in_comment(line))    continue;   // skip comment
            if (is_user_field(line))    continue;   // skip user field
            if (!is_field(line))        continue;   // skip not field line

            field_info f_info;

            f_info.name_            = field_name(line);
            f_info.alias_           = qualifier_alias(line);
            f_info.qualifier_       = field_qualifier(line);
            f_info.qualifier_error_ = false;

            if (f_info.name_.empty()) continue;

            if (qualifier_required(f_info.qualifier_).empty() || qualifier_type(f_info.qualifier_).empty())
            {
                f_info.qualifier_error_ = true;

                lines.insert(iter2, "    #error missing or repeated field qualifier");

                continue;
            }

            if (std::string::npos != f_info.qualifier_.find(ESTR(BASIC_ARRAY))
                || std::string::npos != f_info.qualifier_.find(ESTR(CUSTOM_ARRAY)))
            {
                 if (std::string::npos == line.find("["))
                 {
                     lines.insert(iter2, "    #error expect array field");

                     continue;
                 }
            }

            iter1->fields_.push_back(f_info);

            remove_qualifiers(line);

            // field align
            if (std::string::npos != f_info.qualifier_.find(ESTR(BASIC_ARRAY))
                || std::string::npos != f_info.qualifier_.find(ESTR(CUSTOM_ARRAY)))
            {
                auto iter3 = lines.insert(iter2, (boost::format("    int %1%_size;") % f_info.name_).str());

                iter1->array_size_fields.push_back((boost::format("%1%_size") % f_info.name_).str());

                align(*iter3, line, sregex::compile((boost::format("\\s+(%1%)") % f_info.name_).str()));
            }
        }
    }
}

static void gen_warning_code(std::ofstream& out, const std::string& o_file_name)
{
    out << "/****************************************************************************" << "\n";
    out << "** register struct field code from reading C++ file '" << file_base_name(o_file_name) << ".json.h'" << "\n";
    out << "**" << "\n";
    out << "** created: " << to_simple_string(second_clock::local_time()) << "\n";
    out << "**      by: the json struct compiler version " << compiler_version << "\n";
    out << "**" << "\n";
    out << "** warning! all changes made in this file will be lost!" << "\n";
    out << "*****************************************************************************/" << "\n";
}

static void gen_reg_fields_code(const struct_info& st_info, std::list<std::string>& reg_fields_code)
{
    for (auto iter = st_info.fields_.begin(); iter != st_info.fields_.end(); ++iter)
    {
        if (iter->name_.empty()) continue;

        if (std::string::npos != iter->qualifier_.find(ESTR(ALIAS)))
        {
            if (std::string::npos != iter->qualifier_.find(ESTR(BASIC_ARRAY)))
            {
                reg_fields_code.push_back((boost::format("        %1%_%2%(%3%, %4%, %5%);") % ESTR(JSTRUCT_REG_BASIC_ARRAY_FIELD) % ESTR(ALIAS) % qualifier_required(iter->qualifier_) % iter->name_ % iter->alias_).str());
            }
            else if (std::string::npos != iter->qualifier_.find(ESTR(CUSTOM_ARRAY)))
            {
                reg_fields_code.push_back((boost::format("        %1%_%2%(%3%, %4%, %5%);") % ESTR(JSTRUCT_REG_CUSTOM_ARRAY_FIELD) % ESTR(ALIAS) % qualifier_required(iter->qualifier_) % iter->name_ % iter->alias_).str());
            }
            else if (std::string::npos != iter->qualifier_.find(ESTR(BASIC)))
            {
                reg_fields_code.push_back((boost::format("        %1%_%2%(%3%, %4%, %5%);") % ESTR(JSTRUCT_REG_BASIC_FIELD) % ESTR(ALIAS) % qualifier_required(iter->qualifier_) % iter->name_ % iter->alias_).str());
            }
            else if (std::string::npos != iter->qualifier_.find(ESTR(CUSTOM)))
            {
                reg_fields_code.push_back((boost::format("        %1%_%2%(%3%, %4%, %5%);") % ESTR(JSTRUCT_REG_CUSTOM_FIELD) % ESTR(ALIAS) % qualifier_required(iter->qualifier_) % iter->name_ % iter->alias_).str());
            }
        }
        else
        {
            if (std::string::npos != iter->qualifier_.find(ESTR(BASIC_ARRAY)))
            {
                reg_fields_code.push_back((boost::format("        %1%(%2%, %3%);") % ESTR(JSTRUCT_REG_BASIC_ARRAY_FIELD) % qualifier_required(iter->qualifier_) % iter->name_).str());
            }
            else if (std::string::npos != iter->qualifier_.find(ESTR(CUSTOM_ARRAY)))
            {
                reg_fields_code.push_back((boost::format("        %1%(%2%, %3%);") % ESTR(JSTRUCT_REG_CUSTOM_ARRAY_FIELD) % qualifier_required(iter->qualifier_) % iter->name_).str());
            }
            else if (std::string::npos != iter->qualifier_.find(ESTR(BASIC)))
            {
                reg_fields_code.push_back((boost::format("        %1%(%2%, %3%);") % ESTR(JSTRUCT_REG_BASIC_FIELD) % qualifier_required(iter->qualifier_) % iter->name_).str());
            }
            else if (std::string::npos != iter->qualifier_.find(ESTR(CUSTOM)))
            {
                reg_fields_code.push_back((boost::format("        %1%(%2%, %3%);") % ESTR(JSTRUCT_REG_CUSTOM_FIELD) % qualifier_required(iter->qualifier_) % iter->name_).str());
            }
        }
    }
}

static void align_reg_fields_code(std::list<std::string>& reg_fields_code)
{
    smatch sm;
    int max_qualifier_index = 0;
    std::list<std::string>::iterator max_qualifier_iter;
    sregex qualifier_req_regex = sregex::compile((boost::format("(%1%|%2%)") % ESTR(REQUIRED) % ESTR(OPTIONAL)).str());
    for (auto iter2 = reg_fields_code.begin(); iter2 != reg_fields_code.end(); ++iter2)
    {
        if (!regex_search(*iter2, sm, qualifier_req_regex))
        {
            static boost::format fmt("#error \"missing or error qualifier, only support %1% and %2%\";");

            fmt % ESTR(REQUIRED) % ESTR(OPTIONAL);

            iter2->insert(0, fmt.str().c_str());

            continue;
        }

        auto offset = sm[1].first - iter2->begin();

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
            align(*iter2, *max_qualifier_iter, sregex::compile((boost::format("(%1%|%2%)") % ESTR(REQUIRED) % ESTR(OPTIONAL)).str()));
        }
    }
}

static void gen_init_fields_code(const struct_info& st_info)
{
    for (auto iter2 = st_info.fields_.begin(); iter2 != st_info.fields_.end(); ++iter2)
    {
        if (iter2->name_.empty())         continue;
        if (2 > iter2->qualifier_.size()) continue;

        if (std::string::npos != iter2->qualifier_.find(ESTR(BASIC)) || std::string::npos != iter2->qualifier_.find(ESTR(BASIC_ARRAY)))
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

    if (!out) throw std::logic_error("open '" + o_file_name + "' failed");

    for (auto iter1 = structs.begin(); iter1 != structs.end(); ++iter1)
    {
        auto st_info    = *iter1;
        auto position   = iter1->iter_struct_end_;

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
    gen_warning_code(out, o_file_name);
    for (auto iter = lines.begin(); iter != lines.end(); ++iter)
    {
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

static void parse(std::string i_file_name, std::string out_path, std::string file_ext, bool always)
{
    if ("." == out_path || "./" == out_path || ".\\" == out_path) out_path = path(i_file_name).parent_path().string();

    if (!ends_with(out_path, "/") && !ends_with(out_path, "\\")) out_path += "\\";

    if (!exists(path(out_path))) create_directories(path(out_path));

    std::string o_file_name = out_path + file_base_name(i_file_name);

    replace_last(o_file_name, ".json", file_ext);

    if (!always && !is_out_of_date(i_file_name, o_file_name))
    {
        std::cout << "output is up-to-date" << "\n" << std::endl;

        return;
    }
    else
    {
        std::cout << "output is out-of-date" << "\n" << std::endl;
    }

    read_file(i_file_name);
    read_struct(i_file_name);
    read_fields();

    if (".h" == file_ext)
    {
        write_decl_file(o_file_name);
    }
    else if (".cpp" == file_ext)
    {
        write_impl_file(o_file_name);
    }
}

static void concurrent_parse(const std::vector<std::string>& files, std::string out_path, std::string file_ext, bool always)
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
    std::string onif = cosn(ESTR(input_files), 'i');
    std::string onop = cosn(ESTR(output_path), 'o');
    std::string onab = cosn(ESTR(always_build), 'a');
    std::string onmb = cosn(ESTR(multi_build), 'm');

    odesc.add_options()
        ("help",                                 "show this text and exit")
        ("h_out",                                "generate c++ header file")
        ("cpp_out",                              "generate c++ source file")
        (onif.c_str(), po::value(&input_files),  "the input files that will be build")
        (onop.c_str(), po::value(&output_path),  "save generate c++ header or source files path")
        (onab.c_str(), po::value(&always_build), "is it always build the input files, 1 or 0")
        (onmb.c_str(), po::value(&multi_build),  "is it always build the input files, 1 or 0")
        ;

    podesc.add(ESTR(input_files), -1);

    store(po::command_line_parser(argc, argv).options(odesc).positional(podesc).run(), options);

    notify(options);
}

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

        if (!input_files.size()) throw std::logic_error("the required option '--input_files' is missing");
        if (!output_path)        throw std::logic_error("the required option '--output_path' is missing");

        if (!options.count("h_out") && !options.count("cpp_out"))
        {
            throw std::logic_error("the option '--h_out and --cpp_out' must be given at least one");
        }

        if (options.count("h_out") && options.count("cpp_out"))
        {
            throw std::logic_error("the option '--h_out and --cpp_out' must be given only one");
        }

        if (options.count("h_out"))
        {
            if (*multi_build)
            {
                concurrent_parse(input_files, *output_path, ".h", *always_build);
            }
            else
            {
                BOOST_FOREACH(auto file, input_files)
                {
                    parse(file, *output_path, ".h", *always_build);
                }
            }
        }
        else if (options.count("cpp_out"))
        {
            if (*multi_build)
            {
                concurrent_parse(input_files, *output_path, ".cpp", *always_build);
            }
            else
            {
                BOOST_FOREACH(auto file, input_files)
                {
                    parse(file, *output_path, ".cpp", *always_build);
                }
            }
        }

        return 0;
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << "\n" << std::endl;

        return -1;
    }
}
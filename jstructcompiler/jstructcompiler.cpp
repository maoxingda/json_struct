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
static boost::optional<bool>        always_build(false);   // ignore file last write time
static boost::optional<std::string> input_file;
static boost::optional<std::string> output_file;
static boost::optional<std::string> my_doc_path;
po::options_description             odesc("usage");
version_info                        verinfo;

// sub group
mark_tag alias_name(1);
mark_tag field_name(1);
mark_tag struct_name(1);
mark_tag qualifier_name(1);

// common regex expressions
static const sregex alpha_underscore = (alpha | '_');
static const sregex identifier       = (alpha_underscore >> *(_d | alpha_underscore));

static const sregex qualifier_col1   = (as_xpr(ESTR(REQUIRED)) | ESTR(OPTIONAL));
static const sregex qualifier_col2   = (as_xpr(ESTR(BOOL_T)) | ESTR(NUMBER_T) | ESTR(NUMBER_ARRAY_T) | ESTR(WCHAR_ARRAY_T) | ESTR(WCHAR_TABLE_T) | ESTR(STRUCT_T) | ESTR(STRUCT_ARRAY_T));
static const sregex qualifier_col3   = (as_xpr(ESTR(ALIAS)) >> '(' >> (alias_name = identifier) >> ')');

static const sregex array            = (as_xpr("[") >> +_w >> "]");
static const sregex qualifier        = (qualifier_col1 | qualifier_col2 | qualifier_col3);
static const sregex field            = (bos >> *_s >> repeat<2, 3>(qualifier >> +_s) >> identifier >> +_s >> (field_name = identifier) >> repeat<0, 2>(array) >> *_s >> ';');
static const sregex user_field       = (bos >> +_s >> (s1 = ESTR(USER_T) >> +_s) >> +_w);

static const sregex other_jst        = bos >> (s1 = "#include" >> +_s >> '"' >> +_w) >> ".jst" >> before('"' >> *_s >> eos);

static bool is_user_field(std::string& line)
{
    smatch sm;

    if (regex_search(line, sm, user_field))
    {
        replace_first(line, sm[s1], "");

        return true;
    }

    return false;
}

static bool is_field(std::string& line)
{
    auto len = line.length();

    auto success = regex_search(line, field);

    return success;
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

static std::string parse_struct_name(const std::string& declaration)
{
    smatch sm;

    static const sregex re_struct = as_xpr("struct") >> +_s >> (struct_name = identifier);

    if (regex_search(declaration, sm, re_struct))
    {
        return sm[struct_name];
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
            st_info.iter_struct_beg_ = iter;

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
        iter1->stname_ = parse_struct_name(*iter1->iter_struct_beg_);

        for (auto iter2 = iter1->iter_struct_beg_; iter2 != iter1->iter_struct_end_; ++iter2)
        {
            auto& line = *iter2;

            if (line.empty())           continue;   // skip empty line
            if (is_in_comment(line))    continue;   // skip comment
            if (is_user_field(line))    continue;   // skip user field
            if (!is_field(line))        continue;   // skip not field line

            field_info f_info;

            f_info.name_            = parse_field_name(line);
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

            if (std::string::npos != f_info.qualifier_.find(ESTR(NUMBER_ARRAY_T))
                || std::string::npos != f_info.qualifier_.find(ESTR(WCHAR_ARRAY_T))
                || std::string::npos != f_info.qualifier_.find(ESTR(STRUCT_ARRAY_T))
                )
            {
                if (1 != std::count_if(line.begin(), line.begin() + line.find(';'), [](char c) { return '[' == c; }))
                {
                    lines.insert(iter2, "    #error expect array field");

                    continue;
                }
            }
            else if (std::string::npos != f_info.qualifier_.find(ESTR(WCHAR_TABLE_T)))
            {
                if (2 != std::count_if(line.begin(), line.begin() + line.find(';'), [](char c) { return '[' == c; }))
                {
                    lines.insert(iter2, "    #error expect table field");

                    continue;
                }
            }
            else if (std::string::npos != f_info.qualifier_.find(ESTR(BOOL_T)))
            {
                static const sregex re = as_xpr("bool") >> +_s >> identifier >> *_s >> ';';

                if (!regex_search(line, re))
                {
                    lines.insert(iter2, "    #error expect bool field");

                    continue;
                }
            }
            //else if (std::string::npos != f_info.qualifier_.find(ESTR(NUMBER_T)))
            //{
            //    static const sregex re = sregex::compile("(?:int|unsigned short|unsigned int|long|unsigned long|__int64|float|double)\\s+[a-zA-Z_$][a-zA-Z0-9_$]*\\s*;");

            //    if (!regex_search(line, re))
            //    {
            //        lines.insert(iter2, "    #error expect number field");

            //        continue;
            //    }
            //}

            iter1->fields_.push_back(f_info);

            remove_qualifiers(line);

            // field align
            if (std::string::npos != f_info.qualifier_.find(ESTR(NUMBER_ARRAY_T))
                || std::string::npos != f_info.qualifier_.find(ESTR(WCHAR_TABLE_T))
                || std::string::npos != f_info.qualifier_.find(ESTR(STRUCT_ARRAY_T))
                )
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
        if (iter->name_.empty()) continue;

        if (std::string::npos != iter->qualifier_.find(ESTR(ALIAS)))
        {
            if (std::string::npos != iter->qualifier_.find(ESTR(NUMBER_ARRAY_T)))
            {
                reg_fields_code.push_back((boost::format("        %1%(%2%, %3%, %4%);") % ESTR(JSTRUCT_REG_NUMBER_ARRAY_FIELD_ALIAS) % qualifier_required(iter->qualifier_) % iter->name_ % iter->alias_).str());
            }
            else if (std::string::npos != iter->qualifier_.find(ESTR(WCHAR_TABLE_T)))
            {
                reg_fields_code.push_back((boost::format("        %1%(%2%, %3%, %4%);") % ESTR(JSTRUCT_REG_WCHAR_TABLE_FIELD_ALIAS) % qualifier_required(iter->qualifier_) % iter->name_ % iter->alias_).str());
            }
            else if (std::string::npos != iter->qualifier_.find(ESTR(STRUCT_ARRAY_T)))
            {
                reg_fields_code.push_back((boost::format("        %1%(%2%, %3%, %4%);") % ESTR(JSTRUCT_REG_STRUCT_ARRAY_FIELD_ALIAS) % qualifier_required(iter->qualifier_) % iter->name_ % iter->alias_).str());
            }
            else if (std::string::npos != iter->qualifier_.find(ESTR(BOOL_T)))
            {
                reg_fields_code.push_back((boost::format("        %1%(%2%, %3%, %4%);") % ESTR(JSTRUCT_REG_BOOL_FIELD_ALIAS) % qualifier_required(iter->qualifier_) % iter->name_ % iter->alias_).str());
            }
            else if (std::string::npos != iter->qualifier_.find(ESTR(NUMBER_T)))
            {
                reg_fields_code.push_back((boost::format("        %1%(%2%, %3%, %4%);") % ESTR(JSTRUCT_REG_NUMBER_FIELD_ALIAS) % qualifier_required(iter->qualifier_) % iter->name_ % iter->alias_).str());
            }
            else if (std::string::npos != iter->qualifier_.find(ESTR(WCHAR_ARRAY_T)))
            {
                reg_fields_code.push_back((boost::format("        %1%(%2%, %3%, %4%);") % ESTR(JSTRUCT_REG_WCHAR_ARRAY_FIELD_ALIAS) % qualifier_required(iter->qualifier_) % iter->name_ % iter->alias_).str());
            }
            else if (std::string::npos != iter->qualifier_.find(ESTR(STRUCT_T)))
            {
                reg_fields_code.push_back((boost::format("        %1%(%2%, %3%, %4%);") % ESTR(JSTRUCT_REG_STRUCT_FIELD_ALIAS) % qualifier_required(iter->qualifier_) % iter->name_ % iter->alias_).str());
            }
        }
        else
        {
            if (std::string::npos != iter->qualifier_.find(ESTR(NUMBER_ARRAY_T)))
            {
                reg_fields_code.push_back((boost::format("        %1%(%2%, %3%);") % ESTR(JSTRUCT_REG_NUMBER_ARRAY_FIELD) % qualifier_required(iter->qualifier_) % iter->name_).str());
            }
            else if (std::string::npos != iter->qualifier_.find(ESTR(WCHAR_TABLE_T)))
            {
                reg_fields_code.push_back((boost::format("        %1%(%2%, %3%);") % ESTR(JSTRUCT_REG_WCHAR_TABLE_FIELD) % qualifier_required(iter->qualifier_) % iter->name_).str());
            }
            else if (std::string::npos != iter->qualifier_.find(ESTR(STRUCT_ARRAY_T)))
            {
                reg_fields_code.push_back((boost::format("        %1%(%2%, %3%);") % ESTR(JSTRUCT_REG_STRUCT_ARRAY_FIELD) % qualifier_required(iter->qualifier_) % iter->name_).str());
            }
            else if (std::string::npos != iter->qualifier_.find(ESTR(BOOL_T)))
            {
                reg_fields_code.push_back((boost::format("        %1%(%2%, %3%);") % ESTR(JSTRUCT_REG_BOOL_FIELD) % qualifier_required(iter->qualifier_) % iter->name_).str());
            }
            else if (std::string::npos != iter->qualifier_.find(ESTR(NUMBER_T)))
            {
                reg_fields_code.push_back((boost::format("        %1%(%2%, %3%);") % ESTR(JSTRUCT_REG_NUMBER_FIELD) % qualifier_required(iter->qualifier_) % iter->name_).str());
            }
            else if (std::string::npos != iter->qualifier_.find(ESTR(WCHAR_ARRAY_T)))
            {
                reg_fields_code.push_back((boost::format("        %1%(%2%, %3%);") % ESTR(JSTRUCT_REG_WCHAR_ARRAY_FIELD) % qualifier_required(iter->qualifier_) % iter->name_).str());
            }
            else if (std::string::npos != iter->qualifier_.find(ESTR(STRUCT_T)))
            {
                reg_fields_code.push_back((boost::format("        %1%(%2%, %3%);") % ESTR(JSTRUCT_REG_STRUCT_FIELD) % qualifier_required(iter->qualifier_) % iter->name_).str());
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
        if (iter2->name_.empty())         continue;
        if (2 > iter2->qualifier_.size()) continue;

        if (std::string::npos != iter2->qualifier_.find(ESTR(BOOL_T))
            || std::string::npos != iter2->qualifier_.find(ESTR(NUMBER_T))
            || std::string::npos != iter2->qualifier_.find(ESTR(NUMBER_ARRAY_T))
            || std::string::npos != iter2->qualifier_.find(ESTR(WCHAR_ARRAY_T))
            || std::string::npos != iter2->qualifier_.find(ESTR(WCHAR_TABLE_T))
            )
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

static void parse(std::string i_file_name, std::string o_file_name, std::string file_ext, bool always)
{
    //if ("." == o_file_name || "./" == o_file_name || ".\\" == o_file_name) o_file_name = path(i_file_name).parent_path().string();

    //if (!ends_with(o_file_name, "/") && !ends_with(o_file_name, "\\")) o_file_name += "\\";

    //if (!exists(path(o_file_name))) create_directories(path(o_file_name));

    //o_file_name = o_file_name + file_base_name(i_file_name);

    //replace_last(o_file_name, ".json", file_ext);

    //if (!always && !is_out_of_date(i_file_name, o_file_name))
    //{
    //    std::cout << "output is up-to-date" << "\n" << std::endl;

    //    return;
    //}
    //else
    //{
    //    std::cout << "output is out-of-date" << "\n" << std::endl;
    //}

    read_file(i_file_name);
    parse_structs(i_file_name);
    parse_fields();

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
    std::string onif = cosn(ESTR(input_file), 'i');
    std::string onof = cosn(ESTR(output_file), 'o');
    std::string onab = cosn(ESTR(always_build), 'a');
    std::string onmb = cosn(ESTR(multi_build), 'm');
    std::string mydp = cosn(ESTR(my_doc_path), 'd');

    odesc.add_options()
        ("help,h",                               "show this text and exit")
        ("h_out",                                "generate c++ header file")
        ("cpp_out",                              "generate c++ source file")
        (onif.c_str(), po::value(&input_file),   "the input file that will be build")
        (onof.c_str(), po::value(&output_file),  "generate c++ header or source file name")
        (onab.c_str(), po::value(&always_build), "is it always build the input file, 1 or 0")
        (onmb.c_str(), po::value(&multi_build),  "is it build the input file concurrently, 1 or 0")
        (mydp.c_str(), po::value(&my_doc_path),  "my documents path")
        ;

    store(po::parse_command_line(argc, argv, odesc), options);

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

        if (!input_file)  throw std::logic_error("the required option '--input_file' is missing");
        if (!output_file) throw std::logic_error("the required option '--output_file' is missing");
        if (!my_doc_path) throw std::logic_error("the required option '--my_doc_path' is missing");

        if (!options.count("h_out") && !options.count("cpp_out"))
        {
            throw std::logic_error("the option '--h_out and --cpp_out' must be given at least one");
        }

        if (options.count("h_out") && options.count("cpp_out"))
        {
            throw std::logic_error("the option '--h_out and --cpp_out' must be given only one");
        }

        verinfo.load(*my_doc_path + "\\Addins\\version.xml");

        if (options.count("h_out"))
        {
            parse(*input_file, *output_file, ".h", *always_build);
        }
        else if (options.count("cpp_out"))
        {
            parse(*input_file, *output_file, ".cpp", *always_build);
        }

        return 0;
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << "\n" << std::endl;

        return -1;
    }
}
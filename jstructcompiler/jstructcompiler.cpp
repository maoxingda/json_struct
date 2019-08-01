#include "stdafx.h"
#include "../inc/jmacro.h"
#include "../inc/jqualifier.h"
#include "../jstruct/jstruct/jfield_info.h"
#include "../util/UtilCommonPath.h"

#include <string>
#include <iostream>

#include <boost/format.hpp>
#include <boost/foreach.hpp>
#include <boost/optional.hpp>
#include <boost/xpressive/xpressive.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/filesystem.hpp>
#include <boost/xpressive/regex_actions.hpp>


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

typedef std::list<std::string>::iterator sliter;

struct struct_info
{
    std::string            stname_;
    std::list<field_info>  fields_;
    std::list<std::string> array_size_fields;
    sliter                 iter_struct_beg_;
    sliter                 iter_struct_end_;
    std::list<sliter>      field_qualifiers;
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
static const sregex number              = (as_xpr(ESTR(jint)) | ESTR(juint) | ESTR(jint64) | ESTR(juint64) | ESTR(jfloat) | ESTR(jdouble));

static const sregex qualifier           = (bos >> *_s >> "public" >> +_s >> (qualifier_name = (as_xpr("jrequired") | "joptional")) >> *_s >> ':');

static const sregex field               = (bos >> *_s >> identifier >> +_s >> (field_name = identifier) >> repeat<0, 2>(as_xpr("[") >> (+_d | identifier) >> "]") >> *_s >> ';');

static const sregex bool_field          = (bos >> *_s >> ESTR(jbool) >> +_s >> identifier >> *_s >> ';');

static const sregex number_field        = (bos >> *_s >> number >> +_s >> identifier >> *_s >> ';');
static const sregex number_array_field  = (bos >> *_s >> number >> +_s >> identifier >> repeat<1, 1>(as_xpr("[") >> (+_d | identifier) >> "]") >> *_s >> ';');

static const sregex wchar_array_field   = (bos >> *_s >> ESTR(jwchar) >> +_s >> identifier >> repeat<1, 1>(as_xpr("[") >> (+_d | identifier) >> "]") >> *_s >> ';');
static const sregex wchar_table_field   = (bos >> *_s >> ESTR(jwchar) >> +_s >> identifier >> repeat<2, 2>(as_xpr("[") >> (+_d | identifier) >> "]") >> *_s >> ';');

static const sregex struct_field        = (bos >> *_s >> 'J' >> identifier >> +_s >> identifier >> *_s >> ';');
static const sregex struct_array_field  = (bos >> *_s >> 'J' >> identifier >> +_s >> identifier >> repeat<1, 1>(as_xpr("[") >> (+_d | identifier) >> "]") >> *_s >> ';');

static const sregex other_jst           = (bos >> (s1 = "#include" >> +_s >> '"' >> identifier) >> ".jst" >> before('"' >> *_s >> eos));

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
    smatch what;

    regex_search(line, what, qualifier);

    return what[qualifier_name];
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
    static const sregex re_struct_beg = as_xpr("jstruct") >> +_s >> (struct_name = identifier);

    for (auto iter = lines.begin(); iter != lines.end(); ++iter)
    {
        if (iter->empty() || is_in_comment(*iter)) continue; // skip empty line and comment

        if (regex_search(*iter, sm, re_struct_beg))
        {
            st_info.stname_             = sm[struct_name];
            st_info.iter_struct_beg_    = iter;

            if ("struct_name" != sm[struct_name])
            {
                iter->insert(sm[struct_name].second, jst_base.begin(), jst_base.end());
            }
        }
        else if (regex_search(*iter, re_struct_end))
        {
            st_info.iter_struct_end_ = iter;

            st_info.field_qualifiers.push_back(iter);

            structs.push_back(st_info);

            st_info.field_qualifiers.clear();
        }

        if (!field_qualifier(*iter).empty())
        {
            st_info.field_qualifiers.push_back(iter);
        }
    }

    if (!structs.size()) throw std::logic_error("not find struct in '" + file_name + "'");
}

static void parse_fields()
{
    for (auto iter1 = structs.begin(); iter1 != structs.end(); ++iter1)
    {
        if ("struct_name" == iter1->stname_) continue;

        auto size = 0u;

        for (auto iter2 = iter1->field_qualifiers.begin(); size < iter1->field_qualifiers.size() - 1; ++iter2, ++size)
        {
            std::string section_flag = field_qualifier(**iter2);

            auto iter3 = *iter2; ++iter3;
            auto iter4 =  iter2; ++iter4;
            auto iter5 = *iter4;

            for (; iter3 != iter5; ++iter3)
            {
                auto& line = *iter3;

                if (line.empty())           continue;   // skip empty line
                if (is_in_comment(line))    continue;   // skip comment

                type t = field_type(line);

                if (enum_none == t)
                {
                    lines.insert(iter3, "    #error invalid field type");

                    continue;
                }

                field_info f_info;

                f_info.type_      = t;
                f_info.name_      = parse_field_name(line);
                //f_info.alias_     = qualifier_alias(line);
                f_info.qualifier_ = section_flag;

                iter1->fields_.push_back(f_info);

                // define array size variable
                if (enum_number_array == f_info.type_ || enum_wchar_table == f_info.type_ || enum_struct_array == f_info.type_)
                {
                    auto iter6 = lines.insert(++iter3, (boost::format("    int %1%_size;") % f_info.name_).str());

                    --iter3;

                    iter1->array_size_fields.push_back((boost::format("%1%_size") % f_info.name_).str());

                    sregex re = sregex::compile(f_info.name_);

                    align(*iter6, line, (s1 = re) >> before("_size"), (s1 = re), 1);
                }
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
        if (std::string::npos != iter->qualifier_.find(ESTR(alias)))
        {
            switch (iter->type_)
            {
            case enum_bool:
                {
                    reg_fields_code.push_back((boost::format("        %1%(%2%, %3%, %4%);") % ESTR(JSTRUCT_REG_BOOL_FIELD_ALIAS) % iter->qualifier_ % iter->name_ % iter->alias_).str());
                }
                break;

            case enum_number:
                {
                    reg_fields_code.push_back((boost::format("        %1%(%2%, %3%, %4%);") % ESTR(JSTRUCT_REG_NUMBER_FIELD_ALIAS) % iter->qualifier_ % iter->name_ % iter->alias_).str());
                }
                break;
            case enum_number_array:
                {
                    reg_fields_code.push_back((boost::format("        %1%(%2%, %3%, %4%);") % ESTR(JSTRUCT_REG_NUMBER_ARRAY_FIELD_ALIAS) % iter->qualifier_ % iter->name_ % iter->alias_).str());
                }
                break;

            case enum_wchar_array:
                {
                    reg_fields_code.push_back((boost::format("        %1%(%2%, %3%, %4%);") % ESTR(JSTRUCT_REG_WCHAR_ARRAY_FIELD_ALIAS) % iter->qualifier_ % iter->name_ % iter->alias_).str());
                }
                break;
            case enum_wchar_table:
                {
                    reg_fields_code.push_back((boost::format("        %1%(%2%, %3%, %4%);") % ESTR(JSTRUCT_REG_WCHAR_TABLE_FIELD_ALIAS) % iter->qualifier_ % iter->name_ % iter->alias_).str());
                }
                break;

            case enum_struct:
                {
                    reg_fields_code.push_back((boost::format("        %1%(%2%, %3%, %4%);") % ESTR(JSTRUCT_REG_STRUCT_FIELD_ALIAS) % iter->qualifier_ % iter->name_ % iter->alias_).str());
                }
                break;
            case enum_struct_array:
                {
                    reg_fields_code.push_back((boost::format("        %1%(%2%, %3%, %4%);") % ESTR(JSTRUCT_REG_STRUCT_ARRAY_FIELD_ALIAS) % iter->qualifier_ % iter->name_ % iter->alias_).str());
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
                    reg_fields_code.push_back((boost::format("        %1%(%2%, %3%);") % ESTR(JSTRUCT_REG_BOOL_FIELD) % iter->qualifier_ % iter->name_).str());
                }
                break;

            case enum_number:
                {
                    reg_fields_code.push_back((boost::format("        %1%(%2%, %3%);") % ESTR(JSTRUCT_REG_NUMBER_FIELD) % iter->qualifier_ % iter->name_).str());
                }
                break;
            case enum_number_array:
                {
                    reg_fields_code.push_back((boost::format("        %1%(%2%, %3%);") % ESTR(JSTRUCT_REG_NUMBER_ARRAY_FIELD) % iter->qualifier_ % iter->name_).str());
                }
                break;

            case enum_wchar_array:
                {
                reg_fields_code.push_back((boost::format("        %1%(%2%, %3%);") % ESTR(JSTRUCT_REG_WCHAR_ARRAY_FIELD) % iter->qualifier_ % iter->name_).str());
                }
                break;
            case enum_wchar_table:
                {
                    reg_fields_code.push_back((boost::format("        %1%(%2%, %3%);") % ESTR(JSTRUCT_REG_WCHAR_TABLE_FIELD) % iter->qualifier_ % iter->name_).str());
                }
                break;

            case enum_struct:
                {
                    reg_fields_code.push_back((boost::format("        %1%(%2%, %3%);") % ESTR(JSTRUCT_REG_STRUCT_FIELD) % iter->qualifier_ % iter->name_).str());
                }
                break;
            case enum_struct_array:
                {
                    reg_fields_code.push_back((boost::format("        %1%(%2%, %3%);") % ESTR(JSTRUCT_REG_STRUCT_ARRAY_FIELD) % iter->qualifier_ % iter->name_).str());
                }
                break;
            }
        }
    }
}

static void gen_init_fields_code(const struct_info& st_info)
{
    for (auto iter2 = st_info.fields_.begin(); iter2 != st_info.fields_.end(); ++iter2)
    {
        if (enum_struct != iter2->type_ && enum_struct_array != iter2->type_)
        {
            lines.insert(st_info.iter_struct_end_, (boost::format("        JSTRUCT_INIT_FIELD_ZERO(%1%, %2%);") % st_info.stname_ % iter2->name_).str());
        }
    }

    for (auto iter2 = st_info.array_size_fields.begin(); iter2 != st_info.array_size_fields.end(); ++iter2)
    {
        lines.insert(st_info.iter_struct_end_, (boost::format("        JSTRUCT_INIT_FIELD_ZERO(%1%, %2%);") % st_info.stname_ % *iter2).str());
    }
}

static void align_reg_fields_code(std::list<std::string>& reg_fields_code)
{
    smatch sm;
    int max_qualifier_index = 0;
    sliter max_qualifier_iter;
    static const sregex re_qualifier = (s1 = (as_xpr(ESTR(jrequired)) | ESTR(joptional)));
    for (auto iter2 = reg_fields_code.begin(); iter2 != reg_fields_code.end(); ++iter2)
    {
        if (!regex_search(*iter2, sm, re_qualifier))
        {
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
            align(*iter2, *max_qualifier_iter, re_qualifier, re_qualifier, 1);
        }
    }
}

static void write_decl_file(const std::string& o_file_name)
{
    std::ofstream out(o_file_name);

    for (auto iter1 = structs.begin(); iter1 != structs.end(); ++iter1)
    {
        auto& st_info  = *iter1;
        auto& position = iter1->iter_struct_end_;

        if (!st_info.fields_.size()) continue;

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

        verinfo.load(UtilCommonPath().MyDocuments() + "\\Visual Studio 2010\\Addins\\version.xml");

        parse(*input_file, *output_file);

        return 0;
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << "\n" << std::endl;

        return -1;
    }
}
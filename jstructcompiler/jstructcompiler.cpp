// json2cxxstructHelper.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "jmacro.h"
#include "jqualifier.h"
#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include <boost/xpressive/xpressive.hpp>

using namespace boost::xpressive;
using namespace boost::filesystem;


struct field_info
{
    std::string name_;
    std::string alias_;
    std::string qualifier_;
};

static std::list<field_info>  fields;
static std::list<std::string> lines;

struct register_info
{
    std::string                      sname_;
    std::list<field_info>            fields_;
    std::list<std::string>::iterator iter_struct_beg_;
    std::list<std::string>::iterator iter_struct_end_;
};

static std::string field_qualifier(std::string declaration)
{
    std::string qualifier;
    smatch      qualifier_required;
    smatch      qualifier_type;
    smatch      qualifier_alias;

    static sregex field_qualifier_required_regex = sregex::compile("(REQUIRED|OPTIONA)");
    static sregex field_qualifier_type_regex     = sregex::compile("(BASIC|BASIC_ARRAY|CUSTOM|CUSTOM_ARRAY)");
    static sregex field_qualifier_alias_regex    = sregex::compile("(ALIAS)\\(\\s*[a-zA-Z_$][a-zA-Z0-9_$]*\\s*\\)\\s+");

    if (regex_search(declaration, qualifier_required, field_qualifier_required_regex))
    {
        qualifier += qualifier_required[1];
    }
    if (regex_search(declaration, qualifier_type, field_qualifier_type_regex))
    {
        qualifier += qualifier_type[1];
    }
    if (regex_search(declaration, qualifier_alias, field_qualifier_alias_regex))
    {
        qualifier += qualifier_alias[1];
    }

    return qualifier;
}

static std::string qualifier_required(std::string qualifier)
{
    static smatch req;

    static sregex qualifier_required_regex = sregex::compile("(REQUIRED|OPTIONA)");

    if (regex_search(qualifier, req, qualifier_required_regex))
    {
        return req[1];
    }

    return "";
}

static std::string qualifier_type(std::string qualifier)
{
    static smatch type;

    static sregex qualifier_type_regex = sregex::compile("(BASIC|BASIC_ARRAY|CUSTOM|CUSTOM_ARRAY)");

    if (regex_search(qualifier, type, qualifier_type_regex))
    {
        return type[1];
    }

    return "";
}

static std::string qualifier_alias(std::string qualifier)
{
    static smatch alias;

    static sregex qualifier_alias_regex = sregex::compile("ALIAS\\(([a-zA-Z_$][a-zA-Z0-9_$]*)\\)");

    if (regex_search(qualifier, alias, qualifier_alias_regex))
    {
        return alias[1];
    }

    return "";
}

static std::string field_name(std::string declaration)
{
    smatch name;

    static sregex struct_field_regex = sregex::compile("([a-zA-Z_$][a-zA-Z0-9_$]*)(?:\\[\\d+\\]){0,2}\\s*;");

    if (regex_search(declaration, name, struct_field_regex))
    {
        return name[1];
    }

    return "";
}

static std::string struct_name(std::string declaration)
{
    smatch name;

    if (regex_search(declaration, name, sregex::compile("struct\\s+([a-zA-Z_$][a-zA-Z0-9_$]*)")))
    {
        return name[1];
    }

    return "";
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

static void read_file(std::string in_file_name, std::list<std::string>& lines)
{    
    std::fstream in(in_file_name, std::ios_base::in);

    if (in)
    {
        std::string line;

        while (getline(in, line))
        {
            lines.push_back(line);
        }

        in.close();
    }
}

static void read_struct(std::list<register_info>& reg_infos)
{
    smatch              sm;
    register_info       reg_info;
    static std::string  jstruct_base_str = " : public jstruct_base";

    sregex struct_end_re = sregex::compile("^\\s*\\}\\s*;");
    sregex struct_beg_re = sregex::compile("struct\\s+([a-zA-Z_$][a-zA-Z0-9_$]*)");

    bool in_multiline_comment = false;

    for (auto iter = lines.begin(); iter != lines.end(); ++iter)
    {
        if (iter->empty())                 continue;
        if (is_single_line_comment(*iter)) continue;

        if (is_multiline_comment_beg(*iter))
        {
            in_multiline_comment = true;

            continue;
        }
        else if (is_multiline_comment_end(*iter))
        {
            in_multiline_comment = false;

            continue;
        }
        else if (in_multiline_comment)
        {
            continue;
        }

        if (regex_search(*iter, sm, struct_beg_re))
        {
            reg_info.iter_struct_beg_ = iter;

            iter->insert(sm[1].second, jstruct_base_str.begin(), jstruct_base_str.end());
        }
        else if (regex_search(*iter, struct_end_re))
        {
            reg_info.iter_struct_end_ = iter;

            reg_infos.push_back(reg_info);
        }
    }
}

static void read_fields(std::list<register_info> &reg_infos)
{
    for (auto iter1 = reg_infos.begin(); iter1 != reg_infos.end(); ++iter1)
    {
        std::string st_name = struct_name(*iter1->iter_struct_beg_);

        if (st_name.empty()) continue;

        iter1->sname_ = st_name;

        bool in_multiline_comment = false;

        for (auto iter2 = iter1->iter_struct_beg_; iter2 != iter1->iter_struct_end_; ++iter2)
        {
            if (iter2->empty())                 continue;
            if (is_single_line_comment(*iter2)) continue;

            if (is_multiline_comment_beg(*iter2))
            {
                in_multiline_comment = true;

                continue;
            }
            else if (is_multiline_comment_end(*iter2))
            {
                in_multiline_comment = false;

                continue;
            }
            else if (in_multiline_comment)
            {
                continue;
            }

            field_info f_info;

            f_info.name_      = field_name(*iter2);
            f_info.alias_     = qualifier_alias(*iter2);
            f_info.qualifier_ = field_qualifier(*iter2);

            if (!f_info.name_.empty())
            {
                iter1->fields_.push_back(f_info);

                if (std::string::npos != f_info.qualifier_.find(ESTR(BASIC_ARRAY)) || std::string::npos != f_info.qualifier_.find(ESTR(CUSTOM_ARRAY)))
                {
                    lines.insert(iter1->iter_struct_end_, (boost::format("    int %1%_size;") % f_info.name_).str());
                }
            }
        }
    }
}

static void write_decl_file(std::string out_file_name, std::list<register_info> &reg_infos)
{
    int index = out_file_name.find(".json");
    if (std::string::npos != index) out_file_name.replace(index, 5, "");

    std::fstream out(out_file_name, std::ios_base::out);

    if (out)
    {
        for (auto iter1 = reg_infos.begin(); iter1 != reg_infos.end(); ++iter1)
        {
            lines.insert(iter1->iter_struct_end_, (boost::format("\n    %1%()") % iter1->sname_).str());
            lines.insert(iter1->iter_struct_end_, "    {");
            //////////////////////////////////////////////////////////////////////////
            for (auto iter2 = iter1->fields_.begin(); iter2 != iter1->fields_.end(); ++iter2)
            {
                if (iter2->name_.empty()) continue;

                if (std::string::npos != iter2->qualifier_.find(ESTR(ALIAS)))
                {
                    if (std::string::npos != iter2->qualifier_.find(ESTR(BASIC)))
                    {
                        lines.insert(iter1->iter_struct_end_, (boost::format("        %1%_%2%(%3%, %4%, %5%);") % ESTR(JSTRUCT_REG_BASIC_FIELD) % ESTR(ALIAS) % qualifier_required(iter2->qualifier_) % iter2->name_ % iter2->alias_).str());
                    }
                    else if (std::string::npos != iter2->qualifier_.find(ESTR(CUSTOM)))
                    {
                        lines.insert(iter1->iter_struct_end_, (boost::format("        %1%_%2%(%3%, %4%, %5%);") % ESTR(JSTRUCT_REG_CUSTOM_FIELD) % ESTR(ALIAS) % qualifier_required(iter2->qualifier_) % iter2->name_ % iter2->alias_).str());
                    }
                    else if (std::string::npos != iter2->qualifier_.find(ESTR(BASIC_ARRAY)))
                    {
                        lines.insert(iter1->iter_struct_end_, (boost::format("        %1%_%2%(%3%, %4%, %5%);") % ESTR(JSTRUCT_REG_BASIC_ARRAY_FIELD) % ESTR(ALIAS) % qualifier_required(iter2->qualifier_) % iter2->name_ % iter2->alias_).str());
                    }
                    else if (std::string::npos != iter2->qualifier_.find(ESTR(CUSTOM_ARRAY)))
                    {
                        lines.insert(iter1->iter_struct_end_, (boost::format("        %1%_%2%(%3%, %4%, %5%);") % ESTR(JSTRUCT_REG_CUSTOM_ARRAY_FIELD) % ESTR(ALIAS) % qualifier_required(iter2->qualifier_) % iter2->name_ % iter2->alias_).str());
                    }
                }
                else
                {
                    if (std::string::npos != iter2->qualifier_.find(ESTR(BASIC)))
                    {
                        lines.insert(iter1->iter_struct_end_, (boost::format("        %1%(%2%, %3%);") % ESTR(JSTRUCT_REG_BASIC_FIELD) % qualifier_required(iter2->qualifier_) % iter2->name_).str());
                    }
                    else if (std::string::npos != iter2->qualifier_.find(ESTR(CUSTOM)))
                    {
                        lines.insert(iter1->iter_struct_end_, (boost::format("        %1%(%2%, %3%);") % ESTR(JSTRUCT_REG_CUSTOM_FIELD) % qualifier_required(iter2->qualifier_) % iter2->name_).str());
                    }
                    else if (std::string::npos != iter2->qualifier_.find(ESTR(BASIC_ARRAY)))
                    {
                        lines.insert(iter1->iter_struct_end_, (boost::format("        %1%(%2%, %3%);") % ESTR(JSTRUCT_REG_BASIC_ARRAY_FIELD) % qualifier_required(iter2->qualifier_) % iter2->name_).str());
                    }
                    else if (std::string::npos != iter2->qualifier_.find(ESTR(CUSTOM_ARRAY)))
                    {
                        lines.insert(iter1->iter_struct_end_, (boost::format("        %1%(%2%, %3%);") % ESTR(JSTRUCT_REG_CUSTOM_ARRAY_FIELD) % qualifier_required(iter2->qualifier_) % iter2->name_).str());
                    }
                }
            }
            lines.insert(iter1->iter_struct_end_, "");
            //////////////////////////////////////////////////////////////////////////
            for (auto iter2 = iter1->fields_.begin(); iter2 != iter1->fields_.end(); ++iter2)
            {
                if (iter2->name_.empty())         continue;
                if (2 > iter2->qualifier_.size()) continue;

                if (std::string::npos != iter2->qualifier_.find(ESTR(BASIC)) || std::string::npos != iter2->qualifier_.find(ESTR(BASIC_ARRAY)))
                {
                    lines.insert(iter1->iter_struct_end_, (boost::format("        JSTRUCT_INIT_FIELD_ZERO(%1%, %2%);") % iter1->sname_ % iter2->name_).str());
                }
            }

            lines.insert(iter1->iter_struct_end_, "    }");
        }
        //////////////////////////////////////////////////////////////////////////
        smatch sm;

        static sregex qualifier_regex = sregex::compile("(((REQUIRED|OPTIONAL|BASIC|BASIC_ARRAY|CUSTOM|CUSTOM_ARRAY|ALIAS\\(\\s*\\w+\\s*\\))\\s+){2,3})");

        lines.insert(lines.begin(), "#pragma once");

        for (auto iter = lines.begin(); iter != lines.end(); ++iter)
        {
            if (regex_search(*iter, sm, qualifier_regex))
            {
                iter->replace(sm[1].first, sm[1].second, "");
            }
            out << *iter << "\n";
        }
        out.close();
    }
}

static void write_impl_file(std::string out_file_name, std::string bfile_name, std::list<register_info> &reg_infos)
{

}
//{
//    std::fstream out(out_file_name, std::ios_base::out);
//
//    if (out && !bfile_name.empty())
//    {
//        out << "#include \"stdafx.h\"\n";
//        out << boost::format("#include <%1%>\n\n\n") % bfile_name;
//
//        int count = 0;
//        for (auto iter1 = reg_infos.begin(); iter1 != reg_infos.end(); ++iter1, ++count)
//        {
//            out << boost::format("%1%::%1%()\n") % iter1->sname_;
//            out << "{\n";
//            //////////////////////////////////////////////////////////////////////////
//            for (auto iter2 = iter1->fields_.begin(); iter2 != iter1->fields_.end(); ++iter2)
//            {
//                if (iter2->name_.empty())            continue;
//                if (2 != iter2->qualifier_.size())    continue;
//
//                if (ESTR(BASIC) == iter2->qualifier_[1] || ESTR(CUSTOM) == iter2->qualifier_[1])
//                {
//                    out << boost::format("    JSTRUCT_REG_BASIC_FIELD(%1%, %2%);\n") % iter2->qualifier_[0] % iter2->name_;
//                }
//                else if (ESTR(CUSTOM_ARRAY) == iter2->qualifier_[1])
//                {
//                    out << boost::format("    JSTRUCT_REG_CUSTOM_ARRAY_FIELD(%1%, %2%);\n") % iter2->qualifier_[0] % iter2->name_;
//                }
//            }
//            out << "\n";
//            //////////////////////////////////////////////////////////////////////////
//            for (auto iter2 = iter1->fields_.begin(); iter2 != iter1->fields_.end(); ++iter2)
//            {
//                if (iter2->name_.empty())            continue;
//                if (2 != iter2->qualifier_.size())    continue;
//
//                if (ESTR(BASIC) == iter2->qualifier_[1])
//                {
//                    out << boost::format("    JSTRUCT_INIT_FIELD_ZERO(%1%, %2%);\n") % iter1->sname_ % iter2->name_;
//                }
//            }
//
//            count + 1 == reg_infos.size() ? out << "}\n" : out << "}\n\n";
//        }
//        out.close();
//    }
//}

static void parse(std::string in_file_name, std::string out_file_name)
{
    read_file(in_file_name, lines);

    if (!lines.empty())
    {
        std::list<register_info> reg_infos;

        read_struct(reg_infos);

        read_fields(reg_infos);

        path in_file_path(in_file_name);
        path out_file_path(out_file_name);

        if (".h" == out_file_path.extension().string())
        {
            write_decl_file(out_file_name, reg_infos);
        }
        else if (".cpp" == out_file_path.extension().string())
        {
            write_impl_file(out_file_name, in_file_path.filename().string(), reg_infos);
        }
    }
}

int main(int argc, char *argv[])
{
    if (3 > argc) return -1;

    parse(argv[1], argv[2]);

    return 0;
}
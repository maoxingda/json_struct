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
    bool        qualifier_error_;
};

static std::list<field_info>  fields;
static std::list<std::string> lines;

struct register_info
{
    std::string                      sname_;
    std::list<field_info>            fields_;
    std::list<std::string>           array_size_fields;
    std::list<std::string>::iterator iter_struct_beg_;
    std::list<std::string>::iterator iter_struct_end_;
};

static std::string field_qualifier(std::string declaration)
{
    std::string qualifier;
    smatch      qualifier_required;
    smatch      qualifier_type;
    smatch      qualifier_alias;

    static sregex field_qualifier_required_regex = sregex::compile((boost::format("(%1%|%2%)") % ESTR(REQUIRED) % ESTR(OPTIONAL)).str());
    static sregex field_qualifier_type_regex     = sregex::compile((boost::format("(%1%|%2%|%3%|%4%)") % ESTR(BASIC_ARRAY) % ESTR(CUSTOM_ARRAY) % ESTR(BASIC) % ESTR(CUSTOM)).str());
    static sregex field_qualifier_alias_regex    = sregex::compile((boost::format("(%1%)\\(\\s*[a-zA-Z_$][a-zA-Z0-9_$]*\\s*\\)\\s+") % ESTR(ALIAS)).str());

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
    std::string ret;
    static smatch sm;

    static sregex re = sregex::compile((boost::format("(%1%|%2%)") % ESTR(REQUIRED) % ESTR(OPTIONAL)).str());

    if (regex_search(qualifier, sm, re))
    {
        ret = sm[1];

        if (regex_search(qualifier.substr(sm[1].second - qualifier.begin()), sm, re))
        {
            ret = "";
        }
    }

    return ret;
}

static std::string qualifier_type(std::string qualifier)
{
    std::string ret;
    static smatch sm;

    static sregex re = sregex::compile((boost::format("(%1%|%2%|%3%|%4%)") % ESTR(BASIC_ARRAY) % ESTR(CUSTOM_ARRAY) % ESTR(BASIC) % ESTR(CUSTOM)).str());

    if (regex_search(qualifier, sm, re))
    {
        ret = sm[1];

        if (regex_search(qualifier.substr(sm[1].second - qualifier.begin()), sm, re))
        {
            ret = "";
        }
    }

    return ret;
}

static std::string qualifier_alias(std::string qualifier)
{
    static smatch sm;

    static sregex re = sregex::compile((boost::format("%1%\\(\\s*([a-zA-Z_$][a-zA-Z0-9_$]*)\\s*\\)\\s+") % ESTR(ALIAS)).str());

    if (regex_search(qualifier, sm, re))
    {
        return sm[1];
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

static bool is_in_comment(std::string line)
{
    static bool comment = false;

    if (is_single_line_comment(line)) return true;;

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

static void align(std::string& line1, std::string& line2, std::string name_regex)
{
    smatch sm1;
    smatch sm2;
    sregex re = sregex::compile(name_regex);

    if (regex_search(line1, sm1, re) && regex_search(line2, sm2, re) && sm1[1].first - line1.begin() != sm2[1].first - line2.begin())
    {
        line1.insert(sm1[1].first - line1.begin(), " ");

        align(line1, line2, name_regex);
    }
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

    for (auto iter = lines.begin(); iter != lines.end(); ++iter)
    {
        if (iter->empty() || is_in_comment(*iter)) continue;

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
            if (iter2->empty() || is_in_comment(*iter2)) continue;

            field_info f_info;

            f_info.name_            = field_name(*iter2);
            f_info.alias_           = qualifier_alias(*iter2);
            f_info.qualifier_       = field_qualifier(*iter2);
            f_info.qualifier_error_ = false;

            if (!f_info.name_.empty())
            {
                if (qualifier_required(*iter2).empty() || qualifier_type(*iter2).empty())
                {
                    f_info.qualifier_error_ = true;

                    iter2->insert(0, "#error \"missing or repeated field qualifier\";");
                }

                if (f_info.qualifier_error_) continue;

                iter1->fields_.push_back(f_info);

                smatch sm;

                boost::format qualifier_regex_str("(((%1%|%2%|%3%|%4%|%5%|%6%|%7%\\(\\s*\\w+\\s*\\))\\s+){2,3})");

                qualifier_regex_str % ESTR(REQUIRED) % ESTR(OPTIONAL) % ESTR(BASIC) % ESTR(BASIC_ARRAY) % ESTR(CUSTOM) % ESTR(CUSTOM_ARRAY) % ESTR(ALIAS);

                static sregex qualifier_regex = sregex::compile(qualifier_regex_str.str());

                if (regex_search(*iter2, sm, qualifier_regex))
                {
                    // remove field qualifiers
                    iter2->replace(sm[1].first, sm[1].second, "");
                }

                if (std::string::npos != f_info.qualifier_.find(ESTR(BASIC_ARRAY)) || std::string::npos != f_info.qualifier_.find(ESTR(CUSTOM_ARRAY)))
                {
                    auto iter3 = lines.insert(iter2, (boost::format("    int %1%_size;") % f_info.name_).str());

                    iter1->array_size_fields.push_back((boost::format("%1%_size") % f_info.name_).str());

                    align(*iter3, *iter2, (boost::format("(%1%)") % f_info.name_).str());
                }
            }
        }
    }
}

static void generate_reg_fields_code(std::list<register_info>::iterator iter1, std::list<std::string> &reg_fields)
{
    for (auto iter2 = iter1->fields_.begin(); iter2 != iter1->fields_.end(); ++iter2)
    {
        if (iter2->name_.empty()) continue;

        if (std::string::npos != iter2->qualifier_.find(ESTR(ALIAS)))
        {
            if (std::string::npos != iter2->qualifier_.find(ESTR(BASIC_ARRAY)))
            {
                reg_fields.push_back((boost::format("        %1%_%2%(%3%, %4%, %5%);") % ESTR(JSTRUCT_REG_BASIC_ARRAY_FIELD) % ESTR(ALIAS) % qualifier_required(iter2->qualifier_) % iter2->name_ % iter2->alias_).str());
            }
            else if (std::string::npos != iter2->qualifier_.find(ESTR(CUSTOM_ARRAY)))
            {
                reg_fields.push_back((boost::format("        %1%_%2%(%3%, %4%, %5%);") % ESTR(JSTRUCT_REG_CUSTOM_ARRAY_FIELD) % ESTR(ALIAS) % qualifier_required(iter2->qualifier_) % iter2->name_ % iter2->alias_).str());
            }
            else if (std::string::npos != iter2->qualifier_.find(ESTR(BASIC)))
            {
                reg_fields.push_back((boost::format("        %1%_%2%(%3%, %4%, %5%);") % ESTR(JSTRUCT_REG_BASIC_FIELD) % ESTR(ALIAS) % qualifier_required(iter2->qualifier_) % iter2->name_ % iter2->alias_).str());
            }
            else if (std::string::npos != iter2->qualifier_.find(ESTR(CUSTOM)))
            {
                reg_fields.push_back((boost::format("        %1%_%2%(%3%, %4%, %5%);") % ESTR(JSTRUCT_REG_CUSTOM_FIELD) % ESTR(ALIAS) % qualifier_required(iter2->qualifier_) % iter2->name_ % iter2->alias_).str());
            }
        }
        else
        {
            if (std::string::npos != iter2->qualifier_.find(ESTR(BASIC_ARRAY)))
            {
                reg_fields.push_back((boost::format("        %1%(%2%, %3%);") % ESTR(JSTRUCT_REG_BASIC_ARRAY_FIELD) % qualifier_required(iter2->qualifier_) % iter2->name_).str());
            }
            else if (std::string::npos != iter2->qualifier_.find(ESTR(CUSTOM_ARRAY)))
            {
                reg_fields.push_back((boost::format("        %1%(%2%, %3%);") % ESTR(JSTRUCT_REG_CUSTOM_ARRAY_FIELD) % qualifier_required(iter2->qualifier_) % iter2->name_).str());
            }
            else if (std::string::npos != iter2->qualifier_.find(ESTR(BASIC)))
            {
                reg_fields.push_back((boost::format("        %1%(%2%, %3%);") % ESTR(JSTRUCT_REG_BASIC_FIELD) % qualifier_required(iter2->qualifier_) % iter2->name_).str());
            }
            else if (std::string::npos != iter2->qualifier_.find(ESTR(CUSTOM)))
            {
                reg_fields.push_back((boost::format("        %1%(%2%, %3%);") % ESTR(JSTRUCT_REG_CUSTOM_FIELD) % qualifier_required(iter2->qualifier_) % iter2->name_).str());
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

            std::list<std::string> reg_fields;
            generate_reg_fields_code(iter1, reg_fields);

            // align
            smatch sm;
            int max_qualifier_index = 0;
            std::list<std::string>::iterator max_qualifier_index_iter;
            sregex qualifier_req_regex = sregex::compile((boost::format("(%1%|%2%)") % ESTR(REQUIRED) % ESTR(OPTIONAL)).str());
            for (auto iter2 = reg_fields.begin(); iter2 != reg_fields.end(); ++iter2)
            {
                if (!regex_search(*iter2, sm, qualifier_req_regex)) continue;

                auto offset = sm[1].first - iter2->begin();

                if (max_qualifier_index < offset)
                {
                    max_qualifier_index      = offset;
                    max_qualifier_index_iter = iter2;
                }
            }
            for (auto iter2 = reg_fields.begin(); iter2 != reg_fields.end(); ++iter2)
            {
                if (iter2 != max_qualifier_index_iter)
                {
                    align(*iter2, *max_qualifier_index_iter, (boost::format("(%1%|%2%)") % ESTR(REQUIRED) % ESTR(OPTIONAL)).str());
                }
            }
            for (auto iter2 = reg_fields.begin(); iter2 != reg_fields.end(); ++iter2)
            {
                lines.insert(iter1->iter_struct_end_, *iter2);
            }
            lines.insert(iter1->iter_struct_end_, "");

            // initialization
            for (auto iter2 = iter1->fields_.begin(); iter2 != iter1->fields_.end(); ++iter2)
            {
                if (iter2->name_.empty())         continue;
                if (2 > iter2->qualifier_.size()) continue;

                if (std::string::npos != iter2->qualifier_.find(ESTR(BASIC)) || std::string::npos != iter2->qualifier_.find(ESTR(BASIC_ARRAY)))
                {
                    lines.insert(iter1->iter_struct_end_, (boost::format("        JSTRUCT_INIT_FIELD_ZERO(%1%, %2%);") % iter1->sname_ % iter2->name_).str());
                }
            }
            for (auto iter2 = iter1->array_size_fields.begin(); iter2 != iter1->array_size_fields.end(); ++iter2)
            {
                lines.insert(iter1->iter_struct_end_, (boost::format("        JSTRUCT_INIT_FIELD_ZERO(%1%, %2%);") % iter1->sname_ % *iter2).str());
            }

            lines.insert(iter1->iter_struct_end_, "    }");
        }

        // save
        for (auto iter = lines.begin(); iter != lines.end(); ++iter)
        {
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
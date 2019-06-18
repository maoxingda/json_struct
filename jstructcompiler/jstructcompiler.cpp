// json2cxxstructHelper.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "jmacro.h"
#include "jqualifier.h"
#include <boost/format.hpp>
#include <boost/foreach.hpp>
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

struct struct_info
{
    std::string                      stname_;
    std::list<field_info>            fields_;
    std::list<std::string>           array_size_fields;
    std::list<std::string>::iterator iter_struct_beg_;
    std::list<std::string>::iterator iter_struct_end_;
};

static std::list<std::string> lines;
static std::list<struct_info> structs;

static bool is_user_field(std::string& line)
{
    static smatch sm;
    static sregex re = sregex::compile((boost::format("^\\s+(\\b%1%\\b\\s+)\\w+") % ESTR(USER)).str());

    if (regex_search(line, sm, re))
    {
        line.replace(sm[1].first, sm[1].second, "");

        return true;
    }

    return false;
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
}

static void read_struct()
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
}

static void read_fields()
{
    for (auto iter1 = structs.begin(); iter1 != structs.end(); ++iter1)
    {
        iter1->stname_ = struct_name(*iter1->iter_struct_beg_);

        for (auto iter2 = iter1->iter_struct_beg_; iter2 != iter1->iter_struct_end_; ++iter2)
        {
            auto& line = *iter2;

            if (line.empty())         continue;   // skip empty line
            if (is_in_comment(line))  continue;   // skip comment
            if (is_user_field(line))  continue;   // skip user field

            field_info f_info;

            f_info.name_            = field_name(line);
            f_info.alias_           = qualifier_alias(line);
            f_info.qualifier_       = field_qualifier(line);
            f_info.qualifier_error_ = false;

            if (f_info.name_.empty()) continue;

            if (qualifier_required(line).empty() || qualifier_type(line).empty())
            {
                f_info.qualifier_error_ = true;

                iter2->insert(0, "#error \"missing or repeated field qualifier\";");

                continue;
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

static void write_decl_file(std::string out_file_name)
{
    int index = out_file_name.find(".json");
    if (std::string::npos != index) out_file_name.replace(index, 5, "");

    std::ofstream out(out_file_name);

    if (!out) return;

    for (auto iter1 = structs.begin(); iter1 != structs.end(); ++iter1)
    {
        auto st_info    = *iter1;
        auto position   = iter1->iter_struct_end_;

        // generate register struct fields code in construct function
        lines.insert(position, (boost::format("\n    %1%()") % st_info.stname_).str());
        lines.insert(position, "    {");
        {
            std::list<std::string> reg_fields_code;

            gen_reg_fields_code(st_info, reg_fields_code);

            align_reg_fields_code(reg_fields_code);

            BOOST_FOREACH(auto item, reg_fields_code) lines.insert(position, item);

            // insert empty line
            lines.insert(position, "");

            // fields initialization
            gen_init_fields_code(st_info);
        }
        lines.insert(position, "    }");
    }

    // save
    for (auto iter = lines.begin(); iter != lines.end(); ++iter)
    {
        out << *iter << "\n";
    }
    out.close();
}

static void write_impl_file(std::string out_file_name, std::string bfile_name, std::list<struct_info> &structs)
{
    throw std::logic_error("not implemented!");
}

static std::string file_extension(const std::string& file_name)
{
    return path(file_name).extension().string();
}

static std::string file_base_name(const std::string& file_name)
{
    return path(file_name).filename().string();
}

static void parse(std::string in_file_name, std::string out_file_name)
{
    read_file(in_file_name);

    if (lines.empty()) return;

    read_struct();
    read_fields();

    if (".h" == file_extension(out_file_name))
    {
        write_decl_file(out_file_name);
    }
    else if (".cpp" == file_extension(out_file_name))
    {
        write_impl_file(out_file_name, file_base_name(in_file_name), structs);
    }
}

int main(int argc, char *argv[])
{
    if (3 > argc) return -1;

    parse(argv[1], argv[2]);

    return 0;
}
#include "StdAfx.h"
#include "jreader.h"
#include "jregex.h"
#include <boost/format.hpp>
#include <fstream>
#include "jparse_cmd_arg.h"
#include <jalign.h>
#include <boost/filesystem.hpp>
#include <algorithm>
#include <boost/foreach.hpp>


using namespace boost::filesystem;


static std::string field_qualifier(const std::string& line)
{
    smatch what;

    regex_search(line, what, qualifier);

    return what[qualifier_name];
}

static std::string parse_field_name(const std::string& line)
{
    smatch what;

    if (regex_search(line, what, field))
    {
        return what[field_name];
    }

    return "";
}

type jreader::field_type(const string& line)
{
    smatch what;

    if (regex_search(line, what, bool_field))
    {
        return enum_bool;
    }
    else if (regex_search(line, what, number_field))
    {
        return enum_number;
    }
    else if (regex_search(line, what, number_array_field))
    {
        return enum_number_array;
    }
    else if (regex_search(line, what, wchar_array_field))
    {
        return enum_wchar_array;
    }
    else if (regex_search(line, what, wchar_table_field))
    {
        return enum_wchar_table;
    }
    else if (regex_search(line, what, struct_field))
    {
        if (is_jstruct(what[struct_name]))
        {
            return enum_struct;
        }
    }
    else if (regex_search(line, what, struct_array_field))
    {
        if (is_jstruct(what[struct_name]))
        {
            return enum_struct_array;
        }
    }

    return enum_none;
}

bool jreader::is_jstruct(const std::string& struct_name)
{
    BOOST_FOREACH(auto name, inc_structs_)
    {
        if (name == struct_name)
        {
            return true;
        }
    }

    BOOST_FOREACH(auto st, structs_)
    {
        if (st.stname_ == struct_name)
        {
            return true;
        }
    }

    return false;
}


std::string jreader::search_inc_jst(std::string file_name)
{
    BOOST_FOREACH(auto p, arg_.incs_)
    {
        path file(p + "\\" + file_name);

        if (exists(file))
        {
            return file.string();
        }
    }

    return "";
}

void jreader::read_file(std::string file_name, slist& lines)
{
    std::ifstream in(file_name);

    if (in)
    {
        std::string line;

        while (getline(in, line))
        {
            lines.push_back(line);
        }
    }
    else
    {
        throw std::logic_error("open '" + file_name + "' failed");
    }
}

void jreader::parse_structs()
{
    smatch              what;
    struct_info         st_info;
    std::string         jst_base      = " : public jstruct_base";
    static const sregex re_struct_end = bos >> *_s >> '}' >> *_s >> ';';
    static const sregex re_struct_beg = bos >> *_s >> "jstruct" >> +_s >> (struct_name = identifier);
    static const sregex re_inc_jst    = bos >> *_s >> "#include" >> +_s >> '"' >> (s1 = *(boost::xpressive::set[alnum | (boost::xpressive::set = '.', '/', '\\')]) >> +_w >> ".jst") >> before('"');

    for (auto iter = lines_.begin(); iter != lines_.end(); ++iter)
    {
        if (iter->empty()/* || is_in_comment(*iter)*/) continue; // skip empty line and comment

        if (regex_search(*iter, what, re_inc_jst))
        {
            slist inc_lines;

            std::string file_name = search_inc_jst(what[s1]);

            if (file_name.empty())
            {
                throw std::logic_error("can not find file " + what[s1]);
            }

            read_file(file_name, inc_lines);

            parse_inc_structs(inc_lines);

            continue;
        }
        else if (regex_search(*iter, what, re_struct_beg))
        {
            st_info.stname_             = what[struct_name];
            st_info.iter_struct_beg_    = iter;

            if ("struct_name" != what[struct_name])
            {
                iter->insert(what[struct_name].second, jst_base.begin(), jst_base.end());
            }
        }
        else if (regex_search(*iter, re_struct_end))
        {
            st_info.iter_struct_end_ = iter;

            st_info.field_qualifiers.push_back(iter);

            structs_.push_back(st_info);

            st_info.field_qualifiers.clear();
        }

        if (!field_qualifier(*iter).empty())
        {
            st_info.field_qualifiers.push_back(iter);
        }
    }

    if (!structs_.size()) throw std::logic_error("not find struct in '" + *arg_.input_file + "'");
}

void jreader::parse_inc_structs(slist& lines)
{
    smatch what;

    static const sregex re_struct_beg = bos >> *_s >> "jstruct" >> +_s >> (struct_name = identifier);
    static const sregex re_inc_jst    = bos >> *_s >> "#include" >> +_s >> '"' >> (s1 = *(boost::xpressive::set[alnum | (boost::xpressive::set = '.', '/', '\\')]) >> +_w >> ".jst") >> before('"');

    for (auto iter = lines.begin(); iter != lines.end(); ++iter)
    {
        if (iter->empty()) continue;

        if (regex_search(*iter, what, re_inc_jst))
        {
            slist inc_lines;

            std::string file_name = search_inc_jst(what[s1]);

            if (file_name.empty())
            {
                throw std::logic_error("can not find file " + what[s1]);
            }

            read_file(file_name, inc_lines);

            parse_inc_structs(inc_lines);

            continue;
        }

        if (!regex_search(*iter, what, re_struct_beg)) continue;

        if ("" != what[struct_name] && "struct_name" != what[struct_name])
        {
            inc_structs_.push_back(what[struct_name]);
        }
    }
};

void jreader::parse_fields()
{
    for (auto iter1 = structs_.begin(); iter1 != structs_.end(); ++iter1)
    {
        if ("struct_name" == iter1->stname_) continue;

        auto size = 0u;

        for (auto iter2 = iter1->field_qualifiers.begin(); size < iter1->field_qualifiers.size() - 1; ++iter2, ++size)
        {
            std::string section_flag = field_qualifier(**iter2);

            auto beg = *iter2; ++beg;
            auto end = *(++iter2); --iter2;

            for (auto cur = beg; cur != end; ++cur)
            {
                auto& line = *cur;

                if (line.empty()) continue;                    // skip empty line
                if (regex_match(line, comment_line)) continue; // skip comment line

                type t = field_type(line);

                if (enum_none == t)
                {
                    line.insert(4, "#error unknown field ---> ");

                    continue;
                }

                field_info f_info;

                f_info.type_      = t;
                f_info.name_      = parse_field_name(line);
                f_info.qualifier_ = section_flag;

                iter1->fields_.push_back(f_info);

                // define array size variable
                if (enum_number_array == f_info.type_ || enum_wchar_table == f_info.type_ || enum_struct_array == f_info.type_)
                {
                    auto iter6 = lines_.insert(++cur, (boost::format("    int %1%_size;") % f_info.name_).str());

                    --cur, iter1->array_size_fields.push_back((boost::format("%1%_size") % f_info.name_).str());

                    sregex re = sregex::compile(f_info.name_);

                    jalign().align(*iter6, line, (s1 = re) >> before("_size"), (s1 = re));
                }
            }
        }
    }
}

void jreader::parse()
{
    parse_structs();
    parse_fields();
}

void jreader::concurrent_parse(const std::vector<std::string>& files, std::string out_path)
{
    throw std::logic_error(std::string(__FUNCTION__) + " not implemented!");
}

jreader::jreader(slist& lines
    , std::list<struct_info>& structs
    , jparse_cmd_arg& arg)

    : lines_(lines)
    , structs_(structs)
    , arg_(arg)
{
    read_file(*arg_.input_file, lines_);
    parse();
}

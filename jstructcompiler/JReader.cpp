#include "StdAfx.h"
#include "JReader.h"
#include "JRegexs.h"
#include <boost/format.hpp>
#include <fstream>
#include "JParseCmdArg.h"
#include <JAlign.h>


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
    smatch what;

    if (regex_search(line, what, field))
    {
        return what[field_name];
    }

    return "";
}

void JReader::read_file()
{
    std::ifstream in(*arg_.input_file);

    if (in)
    {
        std::string line;

        while (getline(in, line))
        {
            lines_.push_back(line);
        }
    }
    else
    {
        throw std::logic_error("open '" + *arg_.input_file + "' failed");
    }
}

void JReader::parse_structs()
{
    smatch              what;
    struct_info         st_info;
    std::string         jst_base = " : public jstruct_base";
    static const sregex re_struct_end = bos >> *_s >> '}' >> *_s >> ';';
    static const sregex re_struct_beg = bos >> *_s >> "jstruct" >> +_s >> (struct_name = icase("jst_") >> identifier);

    for (auto iter = lines_.begin(); iter != lines_.end(); ++iter)
    {
        if (iter->empty()/* || is_in_comment(*iter)*/) continue; // skip empty line and comment

        if (regex_search(*iter, what, re_struct_beg))
        {
            st_info.stname_             = what[struct_name];
            st_info.iter_struct_beg_    = iter;

            if ("jst_name" != what[struct_name])
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

void JReader::parse_fields()
{
    for (auto iter1 = structs_.begin(); iter1 != structs_.end(); ++iter1)
    {
        if ("jst_name" == iter1->stname_) continue;

        auto size = 0u;

        for (auto iter2 = iter1->field_qualifiers.begin(); size < iter1->field_qualifiers.size() - 1; ++iter2, ++size)
        {
            std::string section_flag = field_qualifier(**iter2);

            auto beg = *iter2; ++beg;
            auto end = *(++iter2); --iter2;

            for (auto cur = beg; cur != end; ++cur)
            {
                auto& line = *cur;

                if (line.empty()) continue; // skip empty line

                type t = field_type(line);

                if (enum_none == t)
                {
                    cur->insert(4, "#error unknown field ---> ");

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

                    JAlign().align(*iter6, line, (s1 = re) >> before("_size"), (s1 = re));
                }
            }
        }
    }
}

void JReader::parse()
{
    parse_structs();
    parse_fields();
}

void JReader::concurrent_parse(const std::vector<std::string>& files, std::string out_path)
{
    throw std::logic_error(std::string(__FUNCTION__) + " not implemented!");
}

JReader::JReader(std::list<std::string>& lines
    , std::list<struct_info>& structs
    , JParseCmdArg& arg)

    : lines_(lines)
    , structs_(structs)
    , arg_(arg)
{
    read_file();
    parse();
}

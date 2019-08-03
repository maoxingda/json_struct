#include "StdAfx.h"
#include "JWriter.h"
#include <string>
#include <iostream>
#include <boost/format.hpp>
#include <boost/foreach.hpp>
#include <boost/filesystem.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <JVersionManager.h>
#include <boost/xpressive/xpressive.hpp>
#include <boost/xpressive/regex_actions.hpp>
#include "../inc/jmacro.h"
#include "../inc/jqualifier.h"
#include "..\jstruct\jstruct\jfield_info.h"
#include <JAlign.h>
#include "JParseCmdArg.h"


using namespace boost::xpressive;
using namespace boost::filesystem;
using namespace boost::posix_time;


// common regex expressions
static const sregex alpha_underscore    = (alpha | '_');
static const sregex identifier          = (alpha_underscore >> *(_d | alpha_underscore));
static const sregex other_jst           = (bos >> (s1 = "#include" >> +_s >> '"' >> identifier) >> ".jst" >> before('"' >> *_s >> eos));


void JWriter::gen_warning_code(std::ofstream& out)
{
    out << "/****************************************************************************" << "\n";
    out << "** register struct field code from reading file '" << path(*argument_.input_file).filename().string() << "'\n";
    out << "**" << "\n";
    out << "** created: " << to_simple_string(second_clock::local_time()) << "\n";
    out << "**      by: the json struct compiler version " << VersionManager().version_ << "\n";
    out << "**" << "\n";
    out << "** warning! all changes made in this file will be lost!" << "\n";
    out << "*****************************************************************************/" << "\n";
}

void JWriter::gen_reg_fields_code(const struct_info& st_info, std::list<std::string>& reg_fields_code)
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

void JWriter::gen_init_fields_code(const struct_info& st_info)
{
    for (auto iter2 = st_info.fields_.begin(); iter2 != st_info.fields_.end(); ++iter2)
    {
        if (enum_struct != iter2->type_ && enum_struct_array != iter2->type_)
        {
            lines_.insert(st_info.iter_struct_end_, (boost::format("        JSTRUCT_INIT_FIELD_ZERO(%1%, %2%);") % st_info.stname_ % iter2->name_).str());
        }
    }

    for (auto iter2 = st_info.array_size_fields.begin(); iter2 != st_info.array_size_fields.end(); ++iter2)
    {
        lines_.insert(st_info.iter_struct_end_, (boost::format("        JSTRUCT_INIT_FIELD_ZERO(%1%, %2%);") % st_info.stname_ % *iter2).str());
    }
}

void JWriter::align_reg_fields_code(std::list<std::string>& reg_fields_code)
{
    smatch sm;
    int max_qualifier_index = 0;
    sliter max_qualifier_iter;
    static const sregex re_qualifier = (s1 = (as_xpr(ESTR(jreq)) | ESTR(jopt)));
    for (auto iter = reg_fields_code.begin(); iter != reg_fields_code.end(); ++iter)
    {
        if (!regex_search(*iter, sm, re_qualifier))
        {
            continue;
        }

        auto offset = sm[s1].first - iter->begin();

        if (max_qualifier_index < offset)
        {
            max_qualifier_index = offset;
            max_qualifier_iter  = iter;
        }
    }
    for (auto iter = reg_fields_code.begin(); iter != reg_fields_code.end(); ++iter)
    {
        if (iter != max_qualifier_iter)
        {
            JAlign().align(*iter, *max_qualifier_iter, re_qualifier, re_qualifier);
        }
    }
}

void JWriter::write_decl_file()
{
    std::ofstream out(*argument_.output_file);

    for (auto iter1 = structs_.begin(); iter1 != structs_.end(); ++iter1)
    {
        auto& st_info  = *iter1;
        auto& position = iter1->iter_struct_end_;

        if (!st_info.fields_.size()) continue;
        if (st_info.stname_.empty()) continue;

        // generate register field code in construct function
        lines_.insert(position, (boost::format("\n    %1%()") % st_info.stname_).str());
        lines_.insert(position, "    {");
        {
            std::list<std::string> reg_fields_code;

            gen_reg_fields_code(st_info, reg_fields_code);

            align_reg_fields_code(reg_fields_code);

            BOOST_FOREACH(auto item, reg_fields_code) lines_.insert(position, item);

            // insert empty line
            lines_.insert(position, "");

            // field initialization
            gen_init_fields_code(st_info);
        }
        lines_.insert(position, "    }");
    }

    // save
    gen_warning_code(out);

    for (auto iter = lines_.begin(); iter != lines_.end(); ++iter)
    {
        *iter = regex_replace(*iter, other_jst, s1 + ".h");

        out << *iter << "\n";
    }
}

void JWriter::write_impl_file()
{
    throw std::logic_error(std::string(__FUNCTION__) + " not implemented!");
}

JWriter::JWriter(std::list<std::string>& lines
    , std::list<struct_info>& structs
    , JParseCmdArg& arg)

    : lines_(lines)
    , structs_(structs)
    , argument_(arg)
{
}

void JWriter::save()
{
    if (argument_.args_.count("h_out"))
    {
        write_decl_file();
    }
    else if (argument_.args_.count("cpp_out"))
    {
        write_impl_file();
    }
}

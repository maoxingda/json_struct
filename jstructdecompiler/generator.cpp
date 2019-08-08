#include "generator.h"
#include <boost/format.hpp>
#include <boost/foreach.hpp>


#define indent(depth) string().insert(0, 4 * depth, ' ')


generator::generator(const Json::Value& val, const args& arg)
    : json_(val)
    , out_(arg.o_file_name_)
{
    object(json_, 1);
}

void generator::write()
{
    out_ << "#pragma once\n";
    out_ << "#include <jstruct.h>\n\n";

    BOOST_FOREACH(const auto& row, jstructs_)
    {
        out_ << "\n";

        BOOST_FOREACH(const auto& col, row)
        {
            out_ << col;
        }

        out_ << "\n";
    }
}

void generator::object(const Json::Value& obj, unsigned num)
{
    if (obj.isNull() || !obj.isObject()) return;

    vector<string> jstruct;

    boost::format fmt("struct_name_%1%");

    string name = (fmt % num).str();

    jstruct.push_back("jstruct " + name + "\n");
    jstruct.push_back("{\n");
    jstruct.push_back("public jreq:\n");

    BOOST_FOREACH(auto member, obj.getMemberNames())
    {
        switch (obj[member].type())
        {
        case Json::booleanValue:
            jstruct.push_back(indent(1) + "jbool " + member + ";\n");
            break;

        case Json::intValue:
            jstruct.push_back(indent(1) + "jint64 " + member + ";\n");
            break;

        case Json::uintValue:
            jstruct.push_back(indent(1) + "juint64 " + member + ";\n");
            break;

        case Json::realValue:
            jstruct.push_back(indent(1) + "jdouble " + member + ";\n");
            break;

        case Json::stringValue:
            jstruct.push_back(indent(1) + "jwchar " + member + ";\n");
            break;

        case Json::objectValue:
            ++num;
            object(obj[member], num);
            name = (fmt % num).str();
            jstruct.push_back(indent(1) + name + " " + member + ";\n");
            break;

        case Json::arrayValue:
            {
                auto& arrItem = obj[member];

                if (arrItem.isNull() || !arrItem.isArray() || 0 == arrItem.size()) return;

                switch (arrItem[0].type())
                {
                case Json::intValue:
                    jstruct.push_back(indent(1) + "jint64 " + member + "[col];\n");
                    break;

                case Json::uintValue:
                    jstruct.push_back(indent(1) + "juint64 " + member + "[col];\n");
                    break;

                case Json::realValue:
                    jstruct.push_back(indent(1) + "jdouble " + member + "[col];\n");
                    break;

                case Json::stringValue:
                    jstruct.push_back(indent(1) + "jwchar " + member + "[row][col];\n");
                    break;

                case Json::objectValue:
                    ++num;
                    object(arrItem[0], num);
                    string name = (boost::format("struct_name_%1%") % num).str();
                    jstruct.push_back(indent(1) + name + " " + member + "[col];\n");
                    break;
                }
            }
            break;
        }
    }

    jstruct.push_back("};");

    jstructs_.push_back(jstruct);
}

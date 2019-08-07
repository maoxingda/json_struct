#include "generator.h"
#include <boost/format.hpp>
#include <boost/foreach.hpp>


#define indent(depth) string().insert(0, 4 * (depth - 1), ' ')


generator::generator(const Json::Value& val, const args& arg)
    : json_(val)
    , out_(arg.o_file_name_)
{
    out_ << "#include \"jstruct.h\"\n\n\n";

    object(json_, 1, 1);
    array(json_, "", 1, 1);
}

void generator::object(const Json::Value& obj, unsigned depth, unsigned num)
{
    if (obj.isNull() || !obj.isObject()) return;

    boost::format fmt("struct_name_%1%");

    string name = (fmt % num).str();

    out_ << indent(depth) << "jstruct " << name << "\n";
    out_ << indent(depth) << "{\n";

    BOOST_FOREACH(auto member, obj.getMemberNames())
    {
        switch (obj[member].type())
        {
        case Json::booleanValue:
            out_ << indent(depth + 1) << "jbool " << member << ";\n";
            break;

        case Json::intValue:
            out_ << indent(depth + 1) << "jint64 " << member << ";\n";
            break;

        case Json::uintValue:
            out_ << indent(depth + 1) << "juint64 " << member << ";\n";
            break;

        case Json::realValue:
            out_ << indent(depth + 1) << "jdouble " << member << ";\n";
            break;

        case Json::stringValue:
            out_ << indent(depth + 1) << "jwchar " << member << "[2];\n";
            break;

        case Json::objectValue:
            ++num;
            object(obj[member], depth + 1, num);
            name = (fmt % num).str();
            out_ << indent(depth + 1) << name << " " << member << ";\n";
            break;

        case Json::arrayValue:
            {
                auto& preview = obj[member];

                !preview.isNull() && preview.size() && preview[0].isObject() ? ++num : 0;

                array(obj[member], member, depth + 1, num);
            }
            break;
        }
    }

    out_ << indent(depth) << "};\n";
}

void generator::array(const Json::Value& arr, const string& member, unsigned depth, unsigned num)
{
    if (arr.isNull() || !arr.isArray() || 0 == arr.size()) return;

    switch (arr[0].type())
    {
    case Json::intValue:
        out_ << indent(depth) << "jint64 " << member << "[2];\n";
        break;

    case Json::uintValue:
        out_ << indent(depth) << "juint64 " << member << "[2];\n";
        break;

    case Json::realValue:
        out_ << indent(depth) << "jdouble " << member << "[2];\n";
        break;

    case Json::stringValue:
        out_ << indent(depth) << "jwchar " << member << "[2][2];\n";
        break;

    case Json::objectValue:
        object(arr[0], depth, num);
        boost::format fmt("struct_name_%1%");
        string name = (fmt % num).str();
        out_ << indent(depth) << name << " " << member << "[2];\n";
        break;

    //case Json::arrayValue:
    //    break;
    }
}

#include "generator.h"
#include <boost/format.hpp>
#include <boost/foreach.hpp>


#define estr(exp)              #exp
#define indent(depth)          string().insert(0, depth, ' ')
#define calc_max_offset(jtype) max_offset = max_offset < strlen(jtype) ? strlen(jtype) : max_offset


generator::generator(const Json::Value& val, const args& arg)
    : json_(val)
    , out_(arg.o_file_name_, ios::app)
{
}

void generator::write()
{
    object(json_, 1);

    out_ << "#pragma once\n";
    out_ << "#include <jstruct.h>\n\n";

    BOOST_FOREACH(const struct_info& jst, jstructs_)
    {
        out_ << "\n";
        out_ << "jstruct " << jst.stname_ << "\n";
        out_ << "{\n";
        out_ << "public jreq:\n";

        BOOST_FOREACH(const field_info& fld, jst.fields_)
        {
            out_ << indent(4) << fld.type_name_ << indent(jst.max_offset_ - fld.type_name_.length()) << ' ' << fld.name_ << "\n";
        }

        out_ << "};\n";
    }
}

void generator::object(const Json::Value& obj, unsigned num)
{
    if (obj.isNull() || !obj.isObject()) return;

    field_info  fd_info;
    struct_info st_info;

    auto& max_offset = st_info.max_offset_;
    max_offset       = 0;

    boost::format fmt("struct_name_%1%");

    st_info.stname_ = (fmt % num).str();

    BOOST_FOREACH(auto member, obj.getMemberNames())
    {
        fd_info.name_ = member;

        switch (obj[member].type())
        {
        case Json::booleanValue:
            fd_info.type_name_ = estr(jbool);
            fd_info.name_ += ";";
            calc_max_offset(estr(jbool));
            break;

        case Json::intValue:
            fd_info.type_name_ = estr(jint64);
            fd_info.name_ += ";";
            calc_max_offset(estr(jint64));
            break;

        case Json::uintValue:
            fd_info.type_name_ = estr(juint64);
            fd_info.name_ += ";";
            calc_max_offset(estr(juint64));
            break;

        case Json::realValue:
            fd_info.type_name_ = estr(jdouble);
            fd_info.name_ += ";";
            calc_max_offset(estr(jdouble));
            break;

        case Json::stringValue:
            fd_info.type_name_ = estr(jwchar);
            fd_info.name_ += "[512];";
            calc_max_offset(estr(jwchar));
            break;

        case Json::objectValue:
            ++num;
            object(obj[member], num);
            fd_info.type_name_ = (fmt % num).str();
            fd_info.name_ += ";";
            max_offset = max_offset < fd_info.type_name_.length() ? fd_info.type_name_.length() : max_offset;
            break;

        case Json::arrayValue:
            {
                auto& arrItem = obj[member];

                if (arrItem.isNull() || !arrItem.isArray() || 0 == arrItem.size()) return;

                switch (arrItem[0].type())
                {
                case Json::intValue:
                    fd_info.type_name_ = estr(jint64);
                    fd_info.name_ += "[2];";
                    calc_max_offset(estr(jint64));
                    break;

                case Json::uintValue:
                    fd_info.type_name_ = estr(juint64);
                    fd_info.name_ += "[2];";
                    calc_max_offset(estr(juint64));
                    break;

                case Json::realValue:
                    fd_info.type_name_ = estr(jdouble);
                    fd_info.name_ += "[2];";
                    calc_max_offset(estr(jdouble));
                    break;

                case Json::stringValue:
                    fd_info.type_name_ = estr(jwchar);
                    fd_info.name_ += "[2][512];";
                    calc_max_offset(estr(jwchar));
                    break;

                case Json::objectValue:
                    ++num;
                    object(arrItem[0], num);
                    fd_info.type_name_ = (fmt % num).str();
                    fd_info.name_ += "[2];";
                    max_offset = max_offset < fd_info.type_name_.length() ? fd_info.type_name_.length() : max_offset;
                    break;
                }
            }
            break;
        }

        st_info.fields_.push_back(fd_info);
    }

    jstructs_.push_back(st_info);
}

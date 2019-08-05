#include "cJSON.h"
#include <boost/tokenizer.hpp>


static cJSON* resolve_path(cJSON* object, string path)
{
    if (nullptr == object) return nullptr;

    typedef boost::tokenizer< boost::char_separator<char> > tokenizer;

    boost::char_separator<char> sep(".");

    tokenizer token(path, sep);

    for (auto iter = token.begin(); iter != token.end(); ++iter)
    {
        object = cJSON_GetObjectItem(object, iter->c_str());

        if (nullptr == object) return nullptr;
    }

    return object;
}

template <typename T>
T jstruct_base::xpath(string json, string path)
{
    //static_assert(typeid(int) == typeid(T) || typeid(string) == typeid(T));

    if (json.empty())
    {
        if (typeid(int) == typeid(T))
        {
            return 0;
        }
        else if (typeid(string) == typeid(T))
        {
            return "";
        }
    }

    cJSON* root = cJSON_Parse(json.c_str());

    if (nullptr == root)
    {
        if (typeid(int) == typeid(T))
        {
            return 0;
        }
        else if (typeid(string) == typeid(T))
        {
            return "";
        }
    }

    cJSON* item = resolve_path(root, path);

    if (nullptr == item)
    {
        if (typeid(int) == typeid(T))
        {
            return 0;
        }
        else if (typeid(string) == typeid(T))
        {
            return "";
        }
    }

    if (typeid(int) == typeid(T))
    {
        return item->valueint;
    }
    else if (typeid(string) == typeid(T))
    {
        return item->valuestring;
    }

    cJSON_Delete(root);

    return T();
}

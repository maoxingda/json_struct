#pragma once
#include "json.h"
#include <fstream>
#include <args.h>


using namespace std;


class generator
{
public:
    generator(const Json::Value& val, const args& arg);

private:
    void object(const Json::Value& obj, unsigned depth, unsigned num);
    void array(const Json::Value& arr, const string& member, unsigned depth, unsigned num);

private:
    const Json::Value&  json_;
    ofstream            out_;
};


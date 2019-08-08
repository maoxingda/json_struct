#pragma once
#include "json.h"
#include <fstream>
#include <args.h>
#include "align\align.h"
#include "..\jstruct\jstruct\jfield_info.h"


using namespace std;


class generator
{
public:
    generator(const Json::Value& val, const args& arg);

    void write();

private:
    void object(const Json::Value& obj, const string& stname);

private:
    align               align_;
    std::ofstream       out_;
    const Json::Value&  json_;
    vector<struct_info> jstructs_;
};


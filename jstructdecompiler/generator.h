#pragma once
#include "json.h"
#include <fstream>
#include <args.h>


using namespace std;


class generator
{
public:
    generator(const Json::Value& val, const args& arg);

    void write();

private:
    void object(const Json::Value& obj, unsigned num);

private:
    ofstream                 out_;
    const Json::Value&       json_;
    vector< vector<string> > jstructs_;
};


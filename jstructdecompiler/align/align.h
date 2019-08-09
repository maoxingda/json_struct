#pragma once
#include <string>


using namespace std;


class align
{
public:
    align(void);

    void load();
    void save();

    std::string reserve_arr_len_;
    std::string reserve_str_len_;
};


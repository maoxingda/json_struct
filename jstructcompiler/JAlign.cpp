#include "StdAfx.h"
#include "JAlign.h"


void JAlign::align(std::string& to_be_align, std::string& ref, const sregex& re_to_be_align, const sregex& re_ref)
{
    smatch sm1;
    smatch sm2;

    if (regex_search(to_be_align, sm1, re_to_be_align) && regex_search(ref, sm2, re_ref))
    {
        auto offset1 = sm1[s1].second - to_be_align.begin();
        auto offset2 = sm2[s1].second - ref.begin();

        auto offset = offset2 - offset1;

        if (0 >= offset) return;

        to_be_align.insert(sm1[s1].first - to_be_align.begin(), offset, ' ');
    }
}

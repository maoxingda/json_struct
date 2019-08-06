#include "StdAfx.h"
#include "jalign.h"


void jalign::align(std::string& to_be_align, std::string& ref, const sregex& re_to_be_align, const sregex& re_ref)
{
    smatch what1;
    smatch what2;

    if (regex_search(to_be_align, what1, re_to_be_align) && regex_search(ref, what2, re_ref))
    {
        auto offset1 = what1[s1].second - to_be_align.begin();
        auto offset2 = what2[s1].second - ref.begin();

        auto offset = offset2 - offset1;

        if (0 >= offset) return;

        to_be_align.insert(what1[s1].first - to_be_align.begin(), offset, ' ');
    }
}

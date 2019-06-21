#pragma once
#include <string>
using namespace std;


/****************************************************************************
** save derived struct fields information
*****************************************************************************/
struct jstruct_base
{
public:
    jstruct_base();
    jstruct_base(const jstruct_base& other);
    const jstruct_base& operator=(const jstruct_base& other);
    ~jstruct_base();

    bool from_json(string);

private:
    bool from_json_(void*);

protected:
    void register_field(string, string, string, string, void*, void*, int);

private:
    void* d;
};

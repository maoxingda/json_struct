#pragma once
#include <string>


/************************************************************************/
/*          the base struct where save struct fields information        */
/************************************************************************/
struct jstruct_base
{
public:
    jstruct_base();
    jstruct_base(const jstruct_base& other);
    const jstruct_base& operator=(const jstruct_base& other);
    ~jstruct_base();

    bool from_json(std::string);

private:
    bool from_json_(void*);

protected:
    void register_field(std::string, std::string, std::string, std::string, void*, void*, int);

private:
    void* d;
};

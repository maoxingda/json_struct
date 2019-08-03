#include "stdafx.h"
#include "JReader.h"
#include "JWriter.h"
#include <iostream>
#include "JParseCmdArg.h"


int main(int argc, char *argv[])
{
    try
    {
        std::list<std::string> lines;
        std::list<struct_info> structs;
        JParseCmdArg            args(argc, argv);

        JReader reader(lines, structs, args);
        JWriter writer(lines, structs, args);

        writer.save();

        return 0;
    }
    catch (const std::exception& e)
    {
        if (nullptr != e.what() && '\0' != e.what()[0])
        {
            std::cout << e.what() << "\n" << std::endl;
        }

        return -1;
    }
}
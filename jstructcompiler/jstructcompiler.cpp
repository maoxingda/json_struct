#include "stdafx.h"
#include "jreader.h"
#include "jwriter.h"
#include <iostream>
#include "jparse_cmd_arg.h"


int main(int argc, char *argv[])
{
    try
    {
        std::list<std::string> lines;
        std::list<struct_info> structs;
        jparse_cmd_arg         args(argc, argv);

        //for (auto iter = args.incs_.begin(); iter != args.incs_.end(); ++iter)
        //{
        //    std::cout << *iter << std::endl;
        //}

        jreader reader(lines, structs, args);
        jwriter writer(lines, structs, args);

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
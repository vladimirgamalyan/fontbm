#include <iostream>
#include <memory>
#include "App.h"
#include "HelpException.h"

int main(const int argc, char* argv[])
{
    try
    {
        App::execute(argc, argv);
    }
    catch (HelpException&)
    {
        return EXIT_SUCCESS;
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch (...)
    {
        std::cerr << "unknown exception" << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

#include <iostream>
#include <memory>
#include "App.h"

int main(int argc, char* argv[])
{
    try
    {
        std::unique_ptr<App> app(new App);
        app->execute(argc, argv);
    }
    catch (std::exception& e)
    {
        std::cerr << "exception: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch (...)
    {
        std::cerr << "unknown exception" << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

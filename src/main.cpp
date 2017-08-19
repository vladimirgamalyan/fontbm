#include <iostream>
#include <memory>
#include "App.h"
#include "HelpException.h"
#include "freeType/FtLibrary.h"
#include "freeType/FtFace.h"
 
int main(int argc, char* argv[])
{
    try
    {
        ft::Library library;
        ft::Face face(library, "fonts/FreeSans.ttf", 16);
        face.debugInfo();

        std::unique_ptr<App> app(new App);
        app->execute(argc, argv);
    }
    catch (HelpException)
    {
        return EXIT_SUCCESS;
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

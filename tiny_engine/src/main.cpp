#include "application.h"
#include <cstdlib>

int main() {
    //system("cd ..");
    Application app;
    try {
        app.run();
    }
    catch (std::exception& e) {
        std::cout << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
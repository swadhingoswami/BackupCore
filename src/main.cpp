#include "cli/cli.h"
#include <iostream>
#include <cstdlib>

int main(int argc, char* argv[]) {
    try {
        backups::CLI cli;
        return cli.run(argc, argv);
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << "\n";
        return EXIT_FAILURE;
    }
}

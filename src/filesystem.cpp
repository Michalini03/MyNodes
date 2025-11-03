#include "../include/filesystem.h"

int startConsoleProgram() {
    std::string command;
    std::cout << "Welcome to MyNodes Filesystem Console!" << std::endl;
    while (true) {
        std::cout << "MyNodes> ";
        std::getline(std::cin, command);
            if (command == "exit") {
                  std::cout << "Exiting MyNodes Filesystem Console." << std::endl;
                  break;
            } else {
                  std::cout << "Command '" << command << "' not recognized." << std::endl;
            }
    }
    return 1; // success
}
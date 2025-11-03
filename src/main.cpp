#include <iostream>
#include <string>
#include "../include/disk.h"
#include "../include/filesystem.h"

const std::string PATH_TO_DISK = "data/filesystem.dat";

extern int start_program() {
    std::cout << "Starting MyNodes program..." << std::endl;
    // Placeholder for actual program logic
    if (checkIfDiskExists(PATH_TO_DISK)) {
        std::cout << "Disk exists!" << std::endl;
        return startConsoleProgram();
    } else {
        std::cout << "Failed to load the disk! \n EXITING THE PROGRAM!" << std::endl;
        return 0;
    }
}

int main() {
    return start_program();
}
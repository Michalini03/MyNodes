#include <iostream>
#include <limits>
#include <string>
#include "../include/disk.h"
#include "../include/filesystem.h"


int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: ./MyNodes <disk_name>" << std::endl;
        return 1;
    }

    std::string diskName = "data/" + std::string(argv[1]) + ".dat";

    std::cout << "Starting MyNodes program..." << std::endl;
    std::cout << "Target Disk: " << diskName << std::endl;

    if (checkIfDiskExists(diskName)) {
        std::cout << "Disk found. Loading..." << std::endl;
        // In the future, we will verify the Magic Number here to ensure it's valid.
    } else {
        std::cout << "Disk not found. Do you want to format it? (Y/N)" << std::endl;
        char response;
        std::cin >> response;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        if (response != 'Y' && response != 'y') {
            std::cout << "Exiting program. Disk formatting declined." << std::endl;
            return 0;
        }
        formatDisk(diskName);
    }

    return startConsoleProgram();
}
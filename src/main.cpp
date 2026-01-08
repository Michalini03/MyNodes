#include <iostream>
#include <limits>
#include <string>
#include "../include/disk.h"
#include "../include/filesystem.h"
#include "../include/utils.h"


int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: ./MyNodes <disk_name>" << std::endl;
        return 1;
    }

    std::string diskName = "data/" + std::string(argv[1]);

    std::cout << "Starting MyNodes program..." << std::endl;
    std::cout << "Target Disk: " << diskName << std::endl;

    if (checkIfDiskExists(diskName)) {
        std::cout << "Disk found. Loading..." << std::endl;
        mountDisk(diskName);

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
        std::cout << "How much space do you want to allocate for the disk? (e.g., 600MB)" << std::endl;
        std::string sizeArg;
        std::getline(std::cin, sizeArg);
        long long size = parseSize(sizeArg);
        formatDisk(diskName, size);
    }

    return startConsoleProgram();
}
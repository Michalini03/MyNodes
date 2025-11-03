#include "../include/disk.h"
#include <string>
#include <fstream>

bool checkIfDiskExists(const std::string& name) {
    if (FILE *file = fopen(name.c_str(), "r")) {
        fclose(file);
        return true;
    } else {
        return false;
    }   
}
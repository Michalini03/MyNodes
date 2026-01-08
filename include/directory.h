#pragma once
#include <cstdint>

struct DirEntry {
    char name[12];      // fixed-length filename
    int32_t inodeNumber;   // inode it points to
};
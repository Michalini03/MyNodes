#include "../include/disk.h"
#include <iostream>
#include <vector>
#include <cstring>

bool checkIfDiskExists(const std::string& name) {
    std::ifstream file(name);
    return file.good();
}

void formatDisk(const std::string& name) {
    std::cout << "Formatting disk: " << name << "..." << std::endl;

    std::ofstream file(name, std::ios::binary | std::ios::out);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open disk file for formatting!" << std::endl;
        return;
    }

    // 1. Initialize Superblock
    Superblock sb;
    sb.magicNumber = MAGIC_NUMBER;
    sb.blockSize = BLOCK_SIZE;
    sb.totalBlocks = DISK_SIZE / BLOCK_SIZE;
    
    // Heuristic: Let's reserve 10% of blocks for Inodes
    // Note: We will calculate exact inode table size in the next steps, 
    // for now, let's just set up the basic metadata.
    sb.totalInodes = (DISK_SIZE / 10) / sizeof(int); // Rough estimate
    sb.freeBlocks = sb.totalBlocks; 
    sb.freeInodes = sb.totalInodes;

    // 2. Write Superblock to the start of the file
    file.write(reinterpret_cast<const char*>(&sb), sizeof(Superblock));

    // 3. Clear the rest of the disk (write 0s)
    // We create a buffer of one block size filled with zeros
    std::vector<char> emptyBlock(BLOCK_SIZE, 0);

    // We write enough blocks to fill the disk size
    // (Subtract 1 because the superblock takes up part of the first chunk, 
    // but for simplicity in this align-ment, we usually write the SB then pad to block size)
    
    // Move pointer to the beginning of the next block after superblock
    file.seekp(BLOCK_SIZE, std::ios::beg);

    for (int i = 1; i < sb.totalBlocks; ++i) {
        file.write(emptyBlock.data(), BLOCK_SIZE);
    }

    file.close();
    std::cout << "Disk formatted successfully. Size: " << DISK_SIZE / (1024*1024) << " MB." << std::endl;
}
#ifndef DISK_H
#define DISK_H

#include <string>
#include <fstream>
#include <vector>

// 4KB block size is standard
const int BLOCK_SIZE = 4096; 
const int DISK_SIZE = 10 * 1024 * 1024; // Let's start with a 10MB disk
const int MAGIC_NUMBER = 0xAABBCCDD; // A unique ID to recognize our FS

// Offsets for various filesystem structures
const int SUPERBLOCK_OFFSET = 0;
const int INODE_BITMAP_OFFSET = BLOCK_SIZE;
const int DATA_BITMAP_OFFSET = BLOCK_SIZE * 2;
const int INODE_TABLE_OFFSET = BLOCK_SIZE * 3;
const int DATA_OFFSET = BLOCK_SIZE * 13;

const int MAX_INODES = 200;

struct Superblock {
    int magicNumber;       // Identifies the filesystem
    int totalBlocks;       // Total number of blocks on disk
    int totalInodes;       // Total number of inodes
    int freeBlocks;        // Counter of free blocks
    int freeInodes;        // Counter of free inodes
    int blockSize;         // Block size in bytes
};

bool checkIfDiskExists(const std::string& name);
void formatDisk(const std::string& name);

// Data Block Management
int allocateDataBlock();
void saveDataBlock(int blockIndex, const std::vector<char>& data);
void freeDataBlock(int blockIndex);
void initializeRootDirectory();

void mountDisk(const std::string& name);
std::string getDiskName();

#endif
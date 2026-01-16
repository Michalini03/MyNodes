#ifndef DISK_H
#define DISK_H

#include <string>
#include <fstream>
#include <vector>

const int BLOCK_SIZE = 4096; 
const int DISK_SIZE = 10 * 1024 * 1024;
const int MAGIC_NUMBER = 0xAABBCCDD;

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

/**
 * Checks if the virtual disk file exists on the host OS.
 * @param name Filename of the virtual disk.
 * @return True if exists.
 */
bool checkIfDiskExists(const std::string& name);

/**
 * Initializes a new virtual disk file, sets up Superblock, Bitmaps, and Root Directory.
 * @param name Filename of the virtual disk.
 * @param sizeBytes Total size of the disk in bytes.
 */
void formatDisk(const std::string& name, long long sizeBytes);

/**
 * Finds a free data block in the bitmap and marks it as used.
 * @return The block index, or -1 if disk is full.
 */
int allocateDataBlock();

/**
 * Writes data to a specific block on the disk.
 * @param blockIndex The index of the data block.
 * @param data A vector containing the bytes to write (must be BLOCK_SIZE).
 */
void saveDataBlock(int blockIndex, const std::vector<char>& data);

/**
 * Marks a data block as free in the bitmap.
 * @param blockIndex The index to free.
 */
void freeDataBlock(int blockIndex);

/**
 * Sets up Inode 0 as the root directory ("/").
 */
void initializeRootDirectory();

/**
 * Opens an existing virtual disk file and reads the superblock.
 * @param name Filename of the virtual disk.
 */
void mountDisk(const std::string& name);

/**
 * Returns the filename of the currently mounted disk.
 * @return Disk filename.
 */
std::string getDiskName();

#endif
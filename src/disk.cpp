#include "../include/disk.h"
#include "../include/inode.h"
#include "../include/directory.h"
#include <iostream>
#include <vector>
#include <cstring>

bool checkIfDiskExists(const std::string& name) {
    std::ifstream file(name, std::ios::binary);
    if (!file.is_open()) {
            std::cerr << "Error: Could not open disk file." << std::endl;
            return false;
    }

    Superblock sb;
    file.read(reinterpret_cast<char*>(&sb), sizeof(Superblock));
    file.close();

    if (sb.magicNumber != MAGIC_NUMBER) {
        std::cerr << "Error: Invalid Disk Format!" << std::endl;
        return false;
    }

    std::cout << "Disk verification successful. Loading..." << std::endl;
    return true;
}

void formatDisk(const std::string& name, long long sizeBytes) {
    std::cout << "Formatting disk: " << name << "..." << std::endl;

    std::ofstream file(name, std::ios::binary | std::ios::out);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open disk file for formatting!" << std::endl;
        return;
    }

    //Initialize Superblock
    Superblock sb;
    sb.magicNumber = MAGIC_NUMBER;
    sb.blockSize = BLOCK_SIZE;
    sb.totalBlocks = sizeBytes / BLOCK_SIZE;
    sb.totalInodes = MAX_INODES;
    sb.freeBlocks = sb.totalBlocks - 10;
    sb.freeInodes = MAX_INODES;

    file.write(reinterpret_cast<const char*>(&sb), sizeof(Superblock));

    std::vector<char> emptyBlock(BLOCK_SIZE, 0);

    file.seekp(BLOCK_SIZE, std::ios::beg);

    for (int i = 1; i < sb.totalBlocks; ++i) {
        file.write(emptyBlock.data(), BLOCK_SIZE);
    }

    file.close();
    std::cout << "Disk formatted successfully. Size: " << sizeBytes / (1024*1024) << " MB." << std::endl;
    
    mountDisk(name);
    initializeRootDirectory();
}

int allocateDataBlock() {
    std::string diskName = getDiskName();
    std::fstream file(diskName, std::ios::in | std::ios::out | std::ios::binary);
    if (!file.is_open()) return -1;

    Superblock sb;
    file.seekg(0, std::ios::beg);
    file.read(reinterpret_cast<char*>(&sb), sizeof(Superblock));

    if (sb.freeBlocks <= 0) {
        return -1;
    }

    std::vector<char> bitmap(BLOCK_SIZE);
    file.seekg(DATA_BITMAP_OFFSET, std::ios::beg);
    file.read(bitmap.data(), BLOCK_SIZE);

    int freeBlockIndex = -1;
    
    for (int i = 0; i < sb.totalBlocks; ++i) { 
        int byteIndex = i / 8;
        int bitIndex = i % 8;
        
        if (!((bitmap[byteIndex] >> bitIndex) & 1)) {
            freeBlockIndex = i;
            bitmap[byteIndex] |= (1 << bitIndex);
            break;
        }
    }

    if (freeBlockIndex != -1) {
        // Save Bitmap
        file.seekp(DATA_BITMAP_OFFSET, std::ios::beg);
        file.write(bitmap.data(), BLOCK_SIZE);

        // Update Superblock
        sb.freeBlocks--; 
        file.seekp(0, std::ios::beg);
        file.write(reinterpret_cast<char*>(&sb), sizeof(Superblock));
    }
    else {
        std::cerr << "Error: No free data blocks available (Disk Full)!" << std::endl;
    }
    
    file.close();
    return freeBlockIndex;
}

void saveDataBlock(int blockIndex, const std::vector<char>& data) {
    std::string diskName = getDiskName();
    std::ofstream file(diskName, std::ios::in | std::ios::out | std::ios::binary);
    
    if (!file.is_open()) {
        std::cerr << "[ERROR] saveDataBlock failed to open file: " << diskName << std::endl;
        return;
    }

    // Data starts at DATA_OFFSET (Block 13)
    int offset = DATA_OFFSET + (blockIndex * BLOCK_SIZE);

    file.seekp(offset, std::ios::beg);
    if (data.size() >= BLOCK_SIZE) {
         file.write(data.data(), BLOCK_SIZE);
    } 
    else {
        // Pad with zeros if input is too small (safety)
        std::vector<char> padded = data;
        padded.resize(BLOCK_SIZE, 0);
        file.write(padded.data(), BLOCK_SIZE);
    }
    
    file.close();
}

void initializeRootDirectory() {
    int inodeId = allocateInode(); 
    int blockId = allocateDataBlock();

    if (inodeId == -1 || blockId == -1) {
        std::cerr << "Error: Failed to allocate Inode or Data Block for root directory!" << std::endl;
        return;
    }

    if (inodeId != 0 || blockId != 0) {
        std::cerr << "Error: Root directory did not get Inode 0!" << std::endl;
        std::cerr << "Initialization failed.\n Inode ID: " << inodeId << "\n Block ID: " << blockId << std::endl;
        return;
    }

    std::vector<char> blockData(BLOCK_SIZE, 0);
    
    DirEntry dot;
    std::strncpy(dot.name, ".", 12);
    dot.inodeNumber = inodeId;

    DirEntry dotdot;
    std::strncpy(dotdot.name, "..", 12);
    dotdot.inodeNumber = inodeId;

    std::memcpy(blockData.data(), &dot, sizeof(DirEntry));
    std::memcpy(blockData.data() + sizeof(DirEntry), &dotdot, sizeof(DirEntry));

    Inode root = createInode(inodeId, true);
    root.directBlocks[0] = blockId;
    root.size = 2 * sizeof(DirEntry); 
    
    saveInode(root);
    saveDataBlock(blockId, blockData);

    std::cout << "Root directory created (Inode 0)." << std::endl;
}

void freeDataBlock(int blockIndex) {
    std::string diskName = getDiskName();
    std::fstream file(diskName, std::ios::in | std::ios::out | std::ios::binary);
    if (!file.is_open()) return;

    std::vector<char> bitmap(BLOCK_SIZE);
    file.seekg(DATA_BITMAP_OFFSET, std::ios::beg);
    file.read(bitmap.data(), BLOCK_SIZE);

    int byteIndex = blockIndex / 8;
    int bitIndex = blockIndex % 8;
    
    bitmap[byteIndex] &= ~(1 << bitIndex);

    file.seekp(DATA_BITMAP_OFFSET, std::ios::beg);
    file.write(bitmap.data(), BLOCK_SIZE);

    Superblock sb;
    file.seekg(0, std::ios::beg);
    file.read(reinterpret_cast<char*>(&sb), sizeof(Superblock));

    sb.freeBlocks++;

    file.seekp(0, std::ios::beg);
    file.write(reinterpret_cast<char*>(&sb), sizeof(Superblock));
    
    file.close();
}

static std::string mountedDiskName = "";

void mountDisk(const std::string& name) {
    mountedDiskName = name;
}

std::string getDiskName() {
    return mountedDiskName;
}
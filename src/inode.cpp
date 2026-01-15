#include "../include/inode.h"
#include "../include/disk.h"
#include "../include/directory.h"
#include "../include/utils.h"
#include <fstream>
#include <string>
#include <iostream>

Inode createInode(int id, bool isDirectory) {
    Inode inode;
    inode.id = id;
    inode.isDirectory = isDirectory;
    inode.size = 0;
    for (int i = 0; i < 10; ++i) {
        inode.directBlocks[i] = -1;
    }
    inode.singleIndirect = -1;
    inode.doubleIndirect = -1;
    inode.tripleIndirect = -1;
    inode.createdAt = time(nullptr);
    inode.modifiedAt = time(nullptr);
    return inode;
}

void saveInode(const Inode& inode) {
    std::string diskName = getDiskName();
    std::ofstream file(diskName, std::ios::in | std::ios::out | std::ios::binary);
    
    if (!file.is_open()) {
        std::cerr << "[ERROR] saveInode failed to open file: " << diskName << std::endl;
        return;
    };

    // Calculate Offset:
    // Inode Table Start + (ID * Size of Inode)

    int sizeofInode = calculateInodeSize(inode);
    int offset = INODE_TABLE_OFFSET + (inode.id * sizeofInode);

    file.seekp(offset, std::ios::beg);
    file.write(reinterpret_cast<const char*>(&inode), sizeofInode);
    file.close();
}

int allocateInode() {
    std::string diskName = getDiskName();
    std::fstream file(diskName, std::ios::in | std::ios::out | std::ios::binary);
    
    if (!file.is_open()) { 
        std::cerr << "[ERROR] allocateInode failed to open file: " << diskName << std::endl;
        return -1; 
    }

    // Read the Inode Bitmap (Block 1)
    std::vector<char> bitmap(BLOCK_SIZE);
    file.seekg(INODE_BITMAP_OFFSET, std::ios::beg);
    file.read(bitmap.data(), BLOCK_SIZE);

    // 2. Scan for a free bit (0)
    int freeId = -1;
    for (int i = 0; i < MAX_INODES; ++i) {
        int byteIndex = i / 8;
        int bitIndex = i % 8;
        
        // Check if the bit is 0
        char byte = bitmap[byteIndex];
        if (!((byte >> bitIndex) & 1)) {
            freeId = i;
            
            bitmap[byteIndex] |= (1 << bitIndex);
            break;
        }
    }

    if (freeId != -1) {
        file.seekp(INODE_BITMAP_OFFSET, std::ios::beg);
        file.write(bitmap.data(), BLOCK_SIZE);

        Superblock sb;
        file.seekg(0, std::ios::beg);
        file.read(reinterpret_cast<char*>(&sb), sizeof(Superblock));
        
        sb.freeInodes--;
        
        file.seekp(0, std::ios::beg);
        file.write(reinterpret_cast<char*>(&sb), sizeof(Superblock));
    }

    file.close();
    return freeId;
}

void freeInode(int id) {
    std::string diskName = getDiskName();
    std::fstream file(diskName, std::ios::in | std::ios::out | std::ios::binary);
    if (!file.is_open()) return;

    std::vector<char> bitmap(BLOCK_SIZE);
    file.seekg(INODE_BITMAP_OFFSET, std::ios::beg);
    file.read(bitmap.data(), BLOCK_SIZE);

    int byteIndex = id / 8;
    int bitIndex = id % 8;

    bitmap[byteIndex] &= ~(1 << bitIndex);

    file.seekp(INODE_BITMAP_OFFSET, std::ios::beg);
    file.write(bitmap.data(), BLOCK_SIZE);

    Superblock sb;
    file.seekg(0, std::ios::beg);
    file.read(reinterpret_cast<char*>(&sb), sizeof(Superblock));
    
    sb.freeInodes++;
    
    file.seekp(0, std::ios::beg);
    file.write(reinterpret_cast<char*>(&sb), sizeof(Superblock));
    
    file.close();
}

int calculateInodeSize(const Inode& inode) {
    return sizeof(inode);
}

int actualizeInodeTime(Inode& inode) {
    inode.modifiedAt = time(nullptr);
    return 0; // success
}
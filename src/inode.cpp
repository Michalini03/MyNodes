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
        // Write the updated bitmap back to disk
        file.seekp(INODE_BITMAP_OFFSET, std::ios::beg);
        file.write(bitmap.data(), BLOCK_SIZE);
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
    
    file.close();
}

int findInodeId(const std::string& rawName, int currentDirectoryInode) {
    std::string name = deleteTrailingWhitespace(rawName);

    std::string diskName = getDiskName();
    std::fstream file(diskName, std::ios::in | std::ios::out | std::ios::binary);
    if (!file.is_open()) return -1;

    Inode parentInode;
    int parentOffset = INODE_TABLE_OFFSET + (currentDirectoryInode * sizeof(Inode));
    file.seekg(parentOffset, std::ios::beg);
    file.read(reinterpret_cast<char*>(&parentInode), sizeof(Inode));

    int blockIndex = parentInode.directBlocks[0];
    std::vector<char> block(BLOCK_SIZE);
    file.seekg(DATA_OFFSET + (blockIndex * BLOCK_SIZE), std::ios::beg);
    file.read(block.data(), BLOCK_SIZE);

    int maxEntries = BLOCK_SIZE / sizeof(DirEntry);
    for (int i = 0; i < maxEntries; ++i) {
        DirEntry* entry = reinterpret_cast<DirEntry*>(block.data() + (i * sizeof(DirEntry)));
        if (entry->inodeNumber != 0 || entry->name[0] != '\0') {
            if (std::string(entry->name) == name) {
                return entry->inodeNumber;
            }
        }
    }
    return -1;
}

int calculateInodeSize(const Inode& inode) {
    return sizeof(inode);
}

int actualizeInodeTime(Inode& inode) {
    inode.modifiedAt = time(nullptr);
    return 0; // success
}
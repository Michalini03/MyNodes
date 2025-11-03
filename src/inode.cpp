#include "../include/inode.h"

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

int calculateInodeSize(const Inode& inode) {
    return sizeof(inode);
}

int actualizeInodeTime(Inode& inode) {
    inode.modifiedAt = time(nullptr);
    return 0; // success
}
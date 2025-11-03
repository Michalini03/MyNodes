#include <time.h>

struct Inode {
    int id;
    bool isDirectory;
    int size;
    int directBlocks[10];
    int singleIndirect;
    int doubleIndirect;
    int tripleIndirect;
    time_t createdAt;
    time_t modifiedAt;
};

Inode createInode(int id, bool isDirectory);

int calculateInodeSize(const Inode& inode);

int actualizeInodeTime(Inode& inode);
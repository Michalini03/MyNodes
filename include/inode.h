#include <time.h>
#include <string>

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

void saveInode(const Inode& inode);

int allocateInode();

void freeInode(int id);

int calculateInodeSize(const Inode& inode);

int actualizeInodeTime(Inode& inode);
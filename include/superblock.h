const int BLOCK_SIZE = 512;
const int MAX_INODES = 128;
const int MAX_BLOCKS = 1024;

struct SuperBlock {
    int totalInodes;
    int totalBlocks;
    int freeInodes;
    int freeBlocks;
    int blockSize;
    int inodeTableStart;
    int dataBlockStart;
    int rootInode;
};
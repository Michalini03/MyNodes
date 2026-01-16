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

/**
 * Initializes a new Inode struct with default values.
 * @param id The unique ID for this inode.
 * @param isDirectory True if this is a folder, false for a file.
 * @return A fresh Inode struct.
 */
Inode createInode(int id, bool isDirectory);

/**
 * Writes the Inode struct to the virtual disk at the correct offset.
 * @param inode The inode object to save.
 */
void saveInode(const Inode& inode);

/**
 * Finds a free Inode ID in the bitmap and marks it as used.
 * @return The allocated Inode ID, or -1 if full.
 */
int allocateInode();

/**
 * Marks an Inode ID as free in the bitmap.
 * @param id The ID to free.
 */
void freeInode(int id);

/**
 * Traverses the inode's block pointers (direct and indirect) and frees all associated data blocks.
 * @param inode The inode whose data should be cleared.
 */
void freeInodeBlocks(Inode& inode);

/**
 * Calculates the logical size of the inode based on its data.
 * @param inode The target inode.
 * @return Size in bytes.
 */
int calculateInodeSize(const Inode& inode);

/**
 * Updates the 'modifiedAt' timestamp to the current time.
 * @param inode The inode to update.
 * @return The new timestamp.
 */
int actualizeInodeTime(Inode& inode);
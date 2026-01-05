#include "../include/filesystem.h"
#include "../include/disk.h"
#include "../include/inode.h"
#include "../include/directory.h"
#include "../include/utils.h"
#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <algorithm> // For std::isspace

static int currentDirectoryInode = 0; // Start at Root (0)

int startConsoleProgram() {
    std::string command;
    std::cout << "Welcome to MyNodes Filesystem Console!" << std::endl;
    
    while (true) {
        // Show current location in prompt
        std::cout << "MyNodes:/Inode" << currentDirectoryInode << "> ";
        std::getline(std::cin, command);

        if (command == "exit") {
            break;
        } 
        else if (command == "ls") {
            if (command.length() > 3) {
                std::string path = command.substr(3);
                int target = resolvePath(path);
                if (target != -1) listDirectory(target);
                else std::cout << "Path not found." << std::endl;
            } else {
                listDirectory(currentDirectoryInode);
            }
        }
        else if (command.substr(0, 3) == "cd ") {
            std::string dirName = command.substr(3);
            changeDirectory(dirName);
        }
        else if (command.substr(0, 6) == "mkdir ") {
            std::string dirName = command.substr(6);
            if (!dirName.empty()) createDirectory(dirName);
            else std::cout << "Usage: mkdir <dirname>" << std::endl;
        }
        else if (command.substr(0, 3) == "rm ") {
            std::string fileName = command.substr(3);
            deleteFile(fileName);
        }
        else if (command.substr(0, 6) == "touch ") {
            std::string fileName = command.substr(6);
            if (!fileName.empty()) createFile(fileName);
            else std::cout << "Usage: touch <filename>" << std::endl;
        }
        else if (command.substr(0, 3) == "cp ") {
            // Format: cp <host_path> <fs_name>
            std::string args = command.substr(3);
            size_t spacePos = args.find(' ');
            if (spacePos != std::string::npos) {
                std::string hostPath = args.substr(0, spacePos);
                std::string fsName = args.substr(spacePos + 1);
                // Trim fsName
                size_t end = fsName.find_last_not_of(" \t\n\r");
                if (end != std::string::npos) fsName = fsName.substr(0, end + 1);
                copyFile(hostPath, fsName);
            } else {
                std::cout << "Usage: cp <host_path> <dest_filename>" << std::endl;
            }
        }
        else if (command.substr(0, 4) == "cat ") {
            readFile(command.substr(4));
        }
        else if (command.substr(0, 6) == "write ") {
             // Format: write <filename> <content text...>
             std::string args = command.substr(6);
             size_t spacePos = args.find(' ');
             if (spacePos != std::string::npos) {
                 std::string fileName = args.substr(0, spacePos);
                 std::string content = args.substr(spacePos + 1);
                 writeFile(fileName, content);
             } else {
                 std::cout << "Usage: write <filename> <text content>" << std::endl;
             }
        }
        else if (command.substr(0, 6) == "rmdir ") {
            removeDirectory(command.substr(6));
        }
        else if (command.substr(0, 3) == "mv ") {
             std::string args = command.substr(3);
             size_t space = args.find(' ');
             if (space != std::string::npos) {
                 moveFile(args.substr(0, space), args.substr(space + 1));
             } else {
                 std::cout << "Usage: mv <oldname> <newname>" << std::endl;
             }
        }
        else {
            std::cout << "Command '" << command << "' not recognized." << std::endl;
        }
    }
    return 1;
}

int resolvePath(const std::string& path) {
    if (path.empty()) return currentDirectoryInode;

    int currentInodeId = currentDirectoryInode;
    if (path[0] == '/') {
        currentInodeId = 0; 
    }

    std::vector<std::string> tokens = splitPath(path);
    if (tokens.empty() && path[0] == '/') return 0; 

    std::string diskName = getDiskName();
    
    for (const std::string& token : tokens) {
        std::fstream file(diskName, std::ios::in | std::ios::binary);
        Inode inode;
        
        file.seekg(INODE_TABLE_OFFSET + (currentInodeId * sizeof(Inode)), std::ios::beg);
        file.read(reinterpret_cast<char*>(&inode), sizeof(Inode));

        if (!inode.isDirectory) return -1;

        std::vector<char> block(BLOCK_SIZE);
        file.seekg(DATA_OFFSET + (inode.directBlocks[0] * BLOCK_SIZE), std::ios::beg);
        file.read(block.data(), BLOCK_SIZE);

        int nextInodeId = -1;
        int maxEntries = BLOCK_SIZE / sizeof(DirEntry);
        
        for (int i = 0; i < maxEntries; ++i) {
            DirEntry* entry = reinterpret_cast<DirEntry*>(block.data() + (i * sizeof(DirEntry)));
            
            // FIX: Check name[0] instead of inodeNumber != 0
            // Because Inode 0 is valid (Root), but an empty name means empty slot.
            if (entry->name[0] != '\0') {
                if (std::string(entry->name) == token) {
                    nextInodeId = entry->inodeNumber;
                    break;
                }
            }
        }

        if (nextInodeId == -1) return -1;
        currentInodeId = nextInodeId;
    }

    return currentInodeId;
}

std::pair<int, std::string> resolveParentAndName(const std::string& path) {
    std::vector<std::string> tokens = splitPath(path);
    if (tokens.empty()) return {-1, ""};

    std::string fileName = tokens.back();
    tokens.pop_back(); 

    int parentInode = currentDirectoryInode;
    if (path[0] == '/') parentInode = 0; 
    
    std::string diskName = getDiskName();
    
    for (const std::string& token : tokens) {
        std::fstream file(diskName, std::ios::in | std::ios::binary);
        Inode inode;
        file.seekg(INODE_TABLE_OFFSET + (parentInode * sizeof(Inode)), std::ios::beg);
        file.read(reinterpret_cast<char*>(&inode), sizeof(Inode));

        std::vector<char> block(BLOCK_SIZE);
        file.seekg(DATA_OFFSET + (inode.directBlocks[0] * BLOCK_SIZE), std::ios::beg);
        file.read(block.data(), BLOCK_SIZE);

        int nextInode = -1;
        int maxEntries = BLOCK_SIZE / sizeof(DirEntry);
        for (int i = 0; i < maxEntries; ++i) {
            DirEntry* entry = reinterpret_cast<DirEntry*>(block.data() + (i * sizeof(DirEntry)));
            
            // FIX: Same fix here. Trust the name, not the ID.
            if (entry->name[0] != '\0' && std::string(entry->name) == token) {
                nextInode = entry->inodeNumber;
                break;
            }
        }
        if (nextInode == -1) return {-1, ""}; 
        parentInode = nextInode;
    }

    return {parentInode, fileName};
}

void changeDirectory(const std::string& path) {
    int targetInodeId = resolvePath(path);

    if (targetInodeId == -1) {
        std::cout << "Directory not found: " << path << std::endl;
        return;
    }

    std::string diskName = getDiskName();
    std::ifstream file(diskName, std::ios::binary);
    Inode targetInode;
    file.seekg(INODE_TABLE_OFFSET + (targetInodeId * sizeof(Inode)), std::ios::beg);
    file.read(reinterpret_cast<char*>(&targetInode), sizeof(Inode));

    if (!targetInode.isDirectory) {
        std::cout << "'" << path << "' is not a directory." << std::endl;
        return;
    }

    currentDirectoryInode = targetInodeId;
    std::cout << "Changed directory to Inode " << currentDirectoryInode << std::endl;
}

void createDirectory(const std::string& path) {
    std::pair<int, std::string> result = resolveParentAndName(path);
    int parentInodeId = result.first;
    std::string name = result.second;

    if (parentInodeId == -1) {
        std::cerr << "Error: Parent directory not found." << std::endl;
        return;
    }

    int newInodeId = allocateInode();
    int newBlockId = allocateDataBlock();

    if (newInodeId == -1 || newBlockId == -1) {
        std::cerr << "Error: Disk full." << std::endl;
        if (newInodeId != -1) freeInode(newInodeId);
        if (newBlockId != -1) freeDataBlock(newBlockId);
        return;
    }

    // Prepare data block with . and ..
    std::vector<char> blockData(BLOCK_SIZE, 0);

    DirEntry dot;
    std::memset(dot.name, 0, 32);
    std::strncpy(dot.name, ".", 31);
    dot.inodeNumber = newInodeId;

    DirEntry dotdot;
    std::memset(dotdot.name, 0, 32);
    std::strncpy(dotdot.name, "..", 31);
    // CRITICAL: Point ".." to the current directory (the parent)
    dotdot.inodeNumber = parentInodeId; 

    std::memcpy(blockData.data(), &dot, sizeof(DirEntry));
    std::memcpy(blockData.data() + sizeof(DirEntry), &dotdot, sizeof(DirEntry));

    saveDataBlock(newBlockId, blockData);

    Inode newInode = createInode(newInodeId, true);
    newInode.directBlocks[0] = newBlockId;
    newInode.size = 2 * sizeof(DirEntry);
    saveInode(newInode);

    if (addEntryToDirectory(parentInodeId, name, newInodeId)) {
        std::cout << "Directory '" << name << "' created." << std::endl;
    } else {
        std::cerr << "Error: Could not add directory to parent." << std::endl;
        freeInode(newInodeId);
        freeDataBlock(newBlockId);
    }
}

void listDirectory(int inodeId) {
    std::string diskName = getDiskName();
    std::fstream file(diskName, std::ios::in | std::ios::out | std::ios::binary);
    if (!file.is_open()) return;

    Inode inode;
    file.seekg(INODE_TABLE_OFFSET + (inodeId * sizeof(Inode)), std::ios::beg);
    file.read(reinterpret_cast<char*>(&inode), sizeof(Inode));

    int blockIndex = inode.directBlocks[0];
    std::vector<char> block(BLOCK_SIZE);
    file.seekg(DATA_OFFSET + (blockIndex * BLOCK_SIZE), std::ios::beg);
    file.read(block.data(), BLOCK_SIZE);
    
    // Calculate entries based on size, or just scan non-empty slots
    std::cout << "Listing Directory (Inode " << inodeId << "):" << std::endl;
    
    int maxEntries = BLOCK_SIZE / sizeof(DirEntry);
    for (int i = 0; i < maxEntries; ++i) {
        DirEntry* entry = reinterpret_cast<DirEntry*>(block.data() + (i * sizeof(DirEntry)));
        if (entry->inodeNumber != 0 || entry->name[0] != '\0') {
             // Mark directory names with a trailing slash for clarity
             std::cout << "  [" << entry->inodeNumber << "] " << entry->name << std::endl;
        }
    }
}

bool removeDirectory(const std::string& rawName) {
    std::string name = deleteTrailingWhitespace(rawName);

    if (name == "." || name == "..") {
        std::cerr << "Error: Cannot delete system entries." << std::endl;
        return false;
    }

    std::string diskName = getDiskName();
    std::fstream file(diskName, std::ios::in | std::ios::out | std::ios::binary);
    if (!file.is_open()) return false;

    Inode parentInode; // Loading current directory inode
    file.seekg(INODE_TABLE_OFFSET + (currentDirectoryInode * sizeof(Inode)), std::ios::beg);
    file.read(reinterpret_cast<char*>(&parentInode), sizeof(Inode));

    std::vector<char> parentBlock(BLOCK_SIZE);
    int parentBlockOffset = DATA_OFFSET + (parentInode.directBlocks[0] * BLOCK_SIZE);
    file.seekg(parentBlockOffset, std::ios::beg);
    file.read(parentBlock.data(), BLOCK_SIZE);

    int entryIndex = -1;
    int targetInodeId = -1;
    int maxEntries = BLOCK_SIZE / sizeof(DirEntry);

    for (int i = 0; i < maxEntries; ++i) {
        DirEntry* entry = reinterpret_cast<DirEntry*>(parentBlock.data() + (i * sizeof(DirEntry)));
        if (entry->inodeNumber != 0) {
            std::string entryName = std::string(entry->name);
            if (entryName == name) {
                entryIndex = i;
                targetInodeId = entry->inodeNumber;
                break;
            }
        }
    }

    if (entryIndex == -1) {
        std::cerr << "Error: Directory '" << name << "' not found." << std::endl;
        return false;
    }

    // Target is a Directory and is EMPTY
    Inode targetInode;
    file.seekg(INODE_TABLE_OFFSET + (targetInodeId * sizeof(Inode)), std::ios::beg);
    file.read(reinterpret_cast<char*>(&targetInode), sizeof(Inode));

    if (!targetInode.isDirectory) {
        std::cerr << "Error: '" << name << "' is a file. Use rm." << std::endl;
        return false;
    }
    if (targetInode.size > 2 * (int)sizeof(DirEntry)) {
        std::cerr << "Error: Directory is not empty." << std::endl;
        return false;
    }

    freeDataBlock(targetInode.directBlocks[0]);
    freeInode(targetInodeId);

    std::memset(parentBlock.data() + (entryIndex * sizeof(DirEntry)), 0, sizeof(DirEntry));
    file.seekp(parentBlockOffset, std::ios::beg);
    file.write(parentBlock.data(), BLOCK_SIZE);

    std::cout << "Directory '" << name << "' removed." << std::endl;
    return true;
}

void createFile(const std::string& path) {
    std::pair<int, std::string> result = resolveParentAndName(path);
    int parentInodeId = result.first;
    std::string fileName = result.second;

    if (parentInodeId == -1) {
        std::cerr << "Error: Directory path not found." << std::endl;
        return;
    }

    int newInodeId = allocateInode();
    if (newInodeId == -1) {
        std::cerr << "Error: No free inodes." << std::endl;
        return;
    }

    Inode newInode = createInode(newInodeId, false); 
    saveInode(newInode);

    // 3. Add to the RESOLVED parent (not just currentDirectory)
    if (addEntryToDirectory(parentInodeId, fileName, newInodeId)) {
        std::cout << "File '" << fileName << "' created." << std::endl;
    } else {
        freeInode(newInodeId);
    }
}

bool addEntryToDirectory(int parentInodeId, const std::string& rawName, int newInodeId) {
    std::string name = deleteTrailingWhitespace(rawName);

    std::string diskName = getDiskName();
    std::fstream file(diskName, std::ios::in | std::ios::out | std::ios::binary);
    if (!file.is_open()) return false;

    Inode parentInode;
    int parentOffset = INODE_TABLE_OFFSET + (parentInodeId * sizeof(Inode));
    file.seekg(parentOffset, std::ios::beg);
    file.read(reinterpret_cast<char*>(&parentInode), sizeof(Inode));

    int blockIndex = parentInode.directBlocks[0];
    std::vector<char> block(BLOCK_SIZE);
    int blockOffset = DATA_OFFSET + (blockIndex * BLOCK_SIZE);
    
    file.seekg(blockOffset, std::ios::beg);
    file.read(block.data(), BLOCK_SIZE);

    int freeOffset = -1;
    int maxEntries = BLOCK_SIZE / sizeof(DirEntry);
    
    for (int i = 0; i < maxEntries; ++i) {
        DirEntry* entry = reinterpret_cast<DirEntry*>(block.data() + (i * sizeof(DirEntry)));
        bool isOccupied = (entry->inodeNumber != 0) || (entry->name[0] != '\0');
        
        std::string entryName = std::string(entry->name);

        if (isOccupied) { 
            if (entryName == name) {
                std::cerr << "[ERROR] File '" << name << "' already exists." << std::endl;
                freeInode(newInodeId); 
                return false;
            }
        } else {
            if (freeOffset == -1) freeOffset = i * sizeof(DirEntry);
        }
    }

    if (freeOffset == -1) {
        std::cerr << "[ERROR] Directory is full!" << std::endl;
        return false;
    }

    DirEntry newEntry;
    std::memset(newEntry.name, 0, 32); 
    std::strncpy(newEntry.name, name.c_str(), 31); 
    newEntry.inodeNumber = newInodeId;

    std::memcpy(block.data() + freeOffset, &newEntry, sizeof(DirEntry));

    file.seekp(blockOffset, std::ios::beg);
    file.write(block.data(), BLOCK_SIZE);

    parentInode.size += sizeof(DirEntry);
    file.seekp(parentOffset, std::ios::beg);
    file.write(reinterpret_cast<char*>(&parentInode), sizeof(Inode));

    return true;
}

bool deleteFile(const std::string& rawName) {
    std::string name = deleteTrailingWhitespace(rawName);

    if (name == "." || name == "..") {
        std::cerr << "Error: Cannot delete system directories." << std::endl;
        return false;
    }

    std::string diskName = getDiskName();
    std::fstream file(diskName, std::ios::in | std::ios::out | std::ios::binary);
    if (!file.is_open()) return false;

    // --- FIX: USE CURRENT DIRECTORY INODE ---
    Inode parentInode;
    int parentOffset = INODE_TABLE_OFFSET + (currentDirectoryInode * sizeof(Inode));
    file.seekg(parentOffset, std::ios::beg);
    file.read(reinterpret_cast<char*>(&parentInode), sizeof(Inode));

    int blockIndex = parentInode.directBlocks[0];
    int blockOffset = DATA_OFFSET + (blockIndex * BLOCK_SIZE);
    
    std::vector<char> block(BLOCK_SIZE);
    file.seekg(blockOffset, std::ios::beg);
    file.read(block.data(), BLOCK_SIZE);

    int entryIndex = -1;
    int targetInodeId = -1;
    int maxEntries = BLOCK_SIZE / sizeof(DirEntry);

    for (int i = 0; i < maxEntries; ++i) {
        DirEntry* entry = reinterpret_cast<DirEntry*>(block.data() + (i * sizeof(DirEntry)));
        if (entry->inodeNumber != 0) {
            std::string entryName = std::string(entry->name);
            if (entryName == name) {
                entryIndex = i;
                targetInodeId = entry->inodeNumber;
                break;
            }
        }
    }

    if (entryIndex == -1) {
        std::cerr << "Error: File '" << name << "' not found." << std::endl;
        return false;
    }

    Inode targetInode;
    int targetOffset = INODE_TABLE_OFFSET + (targetInodeId * sizeof(Inode));
    file.seekg(targetOffset, std::ios::beg);
    file.read(reinterpret_cast<char*>(&targetInode), sizeof(Inode));

    if (targetInode.isDirectory) {
        std::cerr << "Error: '" << name << "' is a directory. Use rmdir (not implemented yet) or generic rm." << std::endl;
        return false;
    }

    for (int i = 0; i < 10; ++i) {
        if (targetInode.directBlocks[i] != -1) {
            freeDataBlock(targetInode.directBlocks[i]);
        }
    }

    freeInode(targetInodeId);

    std::memset(block.data() + (entryIndex * sizeof(DirEntry)), 0, sizeof(DirEntry));

    file.seekp(blockOffset, std::ios::beg);
    file.write(block.data(), BLOCK_SIZE);

    parentInode.size -= sizeof(DirEntry);
    file.seekp(parentOffset, std::ios::beg);
    file.write(reinterpret_cast<char*>(&parentInode), sizeof(Inode));

    std::cout << "File '" << name << "' deleted." << std::endl;
    return true;
}

bool writeFile(const std::string& fileName, const std::string& content) {
    int inodeId = findInodeId(fileName, currentDirectoryInode);
    if (inodeId == -1) {
        std::cerr << "Error: File '" << fileName << "' not found. Create it first with 'touch'." << std::endl;
        return false;
    }

    std::string diskName = getDiskName();
    std::fstream file(diskName, std::ios::in | std::ios::out | std::ios::binary);
    if (!file.is_open()) return false;

    // 1. Load Inode
    Inode inode;
    int inodeOffset = INODE_TABLE_OFFSET + (inodeId * sizeof(Inode));
    file.seekg(inodeOffset, std::ios::beg);
    file.read(reinterpret_cast<char*>(&inode), sizeof(Inode));

    if (inode.isDirectory) {
        std::cerr << "Error: Cannot write to a directory." << std::endl;
        return false;
    }

    // 2. Calculate blocks needed
    int contentSize = content.size();
    int blocksNeeded = (contentSize + BLOCK_SIZE - 1) / BLOCK_SIZE; // Ceiling division

    if (blocksNeeded > 10) {
        std::cerr << "Error: File too large (Max 40KB for this version)." << std::endl;
        return false;
    }

    // 3. Write Data
    for (int i = 0; i < blocksNeeded; ++i) {
        // Allocate block if not exists
        if (inode.directBlocks[i] == -1) {
            inode.directBlocks[i] = allocateDataBlock();
            if (inode.directBlocks[i] == -1) {
                std::cerr << "Error: Disk full!" << std::endl;
                return false;
            }
        }

        // Prepare chunk
        int pos = i * BLOCK_SIZE;
        int len = std::min(BLOCK_SIZE, contentSize - pos);
        std::string chunk = content.substr(pos, len);

        std::vector<char> buffer(BLOCK_SIZE, 0);
        std::memcpy(buffer.data(), chunk.c_str(), len);

        saveDataBlock(inode.directBlocks[i], buffer);
    }

    // 4. Update Inode Size and Time
    inode.size = contentSize;
    inode.modifiedAt = time(nullptr);
    
    file.seekp(inodeOffset, std::ios::beg);
    file.write(reinterpret_cast<char*>(&inode), sizeof(Inode));

    std::cout << "Wrote " << contentSize << " bytes to '" << fileName << "'." << std::endl;
    return true;
}

void copyFile(const std::string& srcRaw, const std::string& destRaw) {
    // 1. Sanitize Names
    std::string srcName = deleteTrailingWhitespace(srcRaw);
    std::string destName = deleteTrailingWhitespace(destRaw);
    // 2. Find Source
    int srcInodeId = findInodeId(srcName, currentDirectoryInode);
    if (srcInodeId == -1) {
        std::cerr << "Error: Source file '" << srcName << "' not found." << std::endl;
        return;
    }

    // 3. Check Destination
    if (findInodeId(destName, currentDirectoryInode) != -1) {
        std::cerr << "Error: Destination '" << destName << "' already exists." << std::endl;
        return;
    }

    std::string diskName = getDiskName();
    std::fstream file(diskName, std::ios::in | std::ios::out | std::ios::binary);
    if (!file.is_open()) return;

    // 4. Load Source Inode
    Inode srcInode;
    file.seekg(INODE_TABLE_OFFSET + (srcInodeId * sizeof(Inode)), std::ios::beg);
    file.read(reinterpret_cast<char*>(&srcInode), sizeof(Inode));

    if (srcInode.isDirectory) {
        std::cerr << "Error: Cannot copy directories yet (cp -r not implemented)." << std::endl;
        return;
    }

    // 5. Prepare Destination Inode
    int destInodeId = allocateInode();
    if (destInodeId == -1) {
        std::cerr << "Error: No free inodes." << std::endl;
        return;
    }

    Inode destInode = createInode(destInodeId, false);
    destInode.size = srcInode.size; // Copy size
    
    // 6. COPY BLOCKS (The Core Logic)
    // We loop through all 10 direct block pointers
    for (int i = 0; i < 10; ++i) {
        int srcBlockIndex = srcInode.directBlocks[i];

        // If source has data here, we must copy it
        if (srcBlockIndex != -1) {
            // A. Allocate a NEW block for the copy
            int destBlockIndex = allocateDataBlock();
            if (destBlockIndex == -1) {
                std::cerr << "Error: Disk full during copy." << std::endl;
                // (Ideally we should rollback/delete here)
                return;
            }

            // B. Read Source Data
            std::vector<char> buffer(BLOCK_SIZE);
            file.seekg(DATA_OFFSET + (srcBlockIndex * BLOCK_SIZE), std::ios::beg);
            file.read(buffer.data(), BLOCK_SIZE);

            // C. Write to New Destination Block
            file.seekp(DATA_OFFSET + (destBlockIndex * BLOCK_SIZE), std::ios::beg);
            file.write(buffer.data(), BLOCK_SIZE);

            // D. Link new block to new Inode
            destInode.directBlocks[i] = destBlockIndex;
        }
    }

    // 7. Save New Inode
    saveInode(destInode);

    // 8. Add Entry to Directory
    if (addEntryToDirectory(currentDirectoryInode, destName, destInodeId)) {
        std::cout << "File copied from '" << srcName << "' to '" << destName << "'." << std::endl;
    } else {
        std::cerr << "Error linking copied file to directory." << std::endl;
        freeInode(destInodeId);
    }
}

void readFile(const std::string& fileName) {
    int inodeId = findInodeId(fileName, currentDirectoryInode);
    if (inodeId == -1) {
        std::cerr << "Error: File '" << fileName << "' not found." << std::endl;
        return;
    }

    std::string diskName = getDiskName();
    std::fstream file(diskName, std::ios::in | std::ios::out | std::ios::binary);
    if (!file.is_open()) return;

    Inode inode;
    file.seekg(INODE_TABLE_OFFSET + (inodeId * sizeof(Inode)), std::ios::beg);
    file.read(reinterpret_cast<char*>(&inode), sizeof(Inode));

    if (inode.isDirectory) {
        std::cerr << "Error: '" << fileName << "' is a directory." << std::endl;
        return;
    }

    std::cout << "--- Content of " << fileName << " (" << inode.size << " bytes) ---" << std::endl;

    int bytesRemaining = inode.size;
    for (int i = 0; i < 10; ++i) {
        if (inode.directBlocks[i] == -1 || bytesRemaining <= 0) break;

        std::vector<char> buffer(BLOCK_SIZE);
        file.seekg(DATA_OFFSET + (inode.directBlocks[i] * BLOCK_SIZE), std::ios::beg);
        file.read(buffer.data(), BLOCK_SIZE);

        int bytesToRead = std::min(BLOCK_SIZE, bytesRemaining);
        std::cout.write(buffer.data(), bytesToRead); // Print exact bytes
        bytesRemaining -= bytesToRead;
    }
    std::cout << "\n-----------------------------------" << std::endl;
}

void moveFile(const std::string& oldRaw, const std::string& newRaw) {
    // Sanitize both names
    std::string oldName = deleteTrailingWhitespace(oldRaw);

    std::string newName = deleteTrailingWhitespace(newRaw);

    int inodeId = findInodeId(oldName, currentDirectoryInode);
    if (inodeId == -1) {
        std::cerr << "Error: Source '" << oldName << "' not found." << std::endl;
        return;
    }

    if (findInodeId(newName, currentDirectoryInode) != -1) {
        std::cerr << "Error: Destination '" << newName << "' already exists." << std::endl;
        return;
    }

    // Since we are just renaming in the same folder, we can hack this:
    // Just find the entry and change the name string in place.
    
    std::string diskName = getDiskName();
    std::fstream file(diskName, std::ios::in | std::ios::out | std::ios::binary);
    
    // Load Parent Data Block
    Inode parentInode;
    file.seekg(INODE_TABLE_OFFSET + (currentDirectoryInode * sizeof(Inode)), std::ios::beg);
    file.read(reinterpret_cast<char*>(&parentInode), sizeof(Inode));

    std::vector<char> block(BLOCK_SIZE);
    int blockOffset = DATA_OFFSET + (parentInode.directBlocks[0] * BLOCK_SIZE);
    file.seekg(blockOffset, std::ios::beg);
    file.read(block.data(), BLOCK_SIZE);

    int maxEntries = BLOCK_SIZE / sizeof(DirEntry);
    for (int i = 0; i < maxEntries; ++i) {
        DirEntry* entry = reinterpret_cast<DirEntry*>(block.data() + (i * sizeof(DirEntry)));
        if (entry->inodeNumber != 0) {
            if (std::string(entry->name) == oldName) {
                // FOUND IT! Update the name.
                std::memset(entry->name, 0, 32);
                std::strncpy(entry->name, newName.c_str(), 31);
                
                // Write back
                file.seekp(blockOffset, std::ios::beg);
                file.write(block.data(), BLOCK_SIZE);
                
                std::cout << "Renamed '" << oldName << "' to '" << newName << "'." << std::endl;
                return;
            }
        }
    }
}
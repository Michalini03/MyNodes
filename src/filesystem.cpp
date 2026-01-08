#include "../include/filesystem.h"
#include "../include/disk.h"
#include "../include/inode.h"
#include "../include/directory.h"
#include "../include/utils.h"
#include <iostream>
#include <string>
#include <sstream>
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

        if (!executeCommand(command)) {
            break;
        }
    }

    return 0;
}

bool executeCommand(std::string commandLine) {
    if (commandLine.empty()) return true;

    std::stringstream ss(commandLine);
    std::string cmd;
    ss >> cmd;

    // Helper string to check for extra arguments
    std::string extra; 

    if (cmd == "exit") {
        return false;
    }
    else if (cmd == "format") {
        long long sizeVal;
        std::string strVal;
        if (ss >> strVal) {
            if (ss >> extra) {
                std::cout << "Error: Too many arguments. Usage: format <size_in_bytes>" << std::endl;
            } else {
                sizeVal = parseSize(strVal);
                formatDisk(getDiskName(), sizeVal);
            }
        } else {
            std::cout << "Usage: format <size_in_bytes>" << std::endl;
        }
    }
    else if (cmd == "ls") {
        std::string path;
        if (ss >> path) {
            if (ss >> extra) {
                std::cout << "Error: Too many arguments. Usage: ls [path]" << std::endl;
            } else {
                int target = resolvePath(path);
                if (target != -1) listDirectory(target);
                else std::cout << "Path not found." << std::endl;
            }
        } else {
            listDirectory(currentDirectoryInode);
        }
    }
    else if (cmd == "cd") {
        std::string dirName;
        if (ss >> dirName) {
            if (ss >> extra) std::cout << "Error: Too many arguments. Usage: cd <dirname>" << std::endl;
            else changeDirectory(dirName);
        } 
        else std::cout << "Usage: cd <dirname>" << std::endl;
    }
    else if (cmd == "mkdir") {
        std::string dirName;
        if (ss >> dirName) {
            if (ss >> extra) std::cout << "Error: Too many arguments. Usage: mkdir <dirname>" << std::endl;
            else createDirectory(dirName);
        } 
        else std::cout << "Usage: mkdir <dirname>" << std::endl;
    }
    else if (cmd == "rm") {
        std::string fileName;
        if (ss >> fileName) {
            if (ss >> extra) std::cout << "Error: Too many arguments. Usage: rm <filename>" << std::endl;
            else deleteFile(fileName);
        } 
        else std::cout << "Usage: rm <filename>" << std::endl;
    }
    else if (cmd == "rmdir") {
        std::string dirName;
        if (ss >> dirName) {
            if (ss >> extra) std::cout << "Error: Too many arguments. Usage: rmdir <dirname>" << std::endl;
            else removeDirectory(dirName);
        } 
        else std::cout << "Usage: rmdir <dirname>" << std::endl;
    }
    else if (cmd == "touch") {
        std::string arg;
        if (ss >> arg) {
            if (ss >> extra) {
                std::cout << "Error: Too many arguments. Usage: touch <filename>" << std::endl;
            } else {
                std::pair<int, std::string> p = resolveParentAndName(arg);
                if (isValid83(p.second)) {
                    createFile(arg);
                } else {
                    std::cerr << "Error: Name '" << p.second << "' is not valid format." << std::endl;
                }
            }
        } else {
            std::cout << "Usage: touch <filename>" << std::endl;
        }
    }
    else if (cmd == "cat") {
        std::string fileName;
        if (ss >> fileName) {
            if (ss >> extra) std::cout << "Error: Too many arguments. Usage: cat <filename>" << std::endl;
            else readFile(fileName);
        } 
        else std::cout << "Usage: cat <filename>" << std::endl;
    }
    else if (cmd == "info") {
        std::string fileName;
        if (ss >> fileName) {
            if (ss >> extra) std::cout << "Error: Too many arguments. Usage: info <filename>" << std::endl;
            else showFileStats(fileName);
        } else {
            std::cout << "Usage: info <filename>" << std::endl;
        }
    }
    else if (cmd == "statfs") {
        if (ss >> extra) {
            std::cout << "Error: 'statfs' takes no arguments." << std::endl;
        } else {
            showDiskInfo();
        }
    }
    else if (cmd == "cp") {
        std::string src, dest;
        if (ss >> src >> dest) {
            if (ss >> extra) std::cout << "Error: Too many arguments. Usage: cp <src> <dest>" << std::endl;
            else copyFile(src, dest);
        } else {
            std::cout << "Usage: cp <source> <destination>" << std::endl;
        }
    }
    else if (cmd == "mv") {
        std::string oldName, newName;
        if (ss >> oldName >> newName) {
            if (ss >> extra) std::cout << "Error: Too many arguments. Usage: mv <old> <new>" << std::endl;
            else moveFile(oldName, newName);
        } else {
            std::cout << "Usage: mv <oldname> <newname>" << std::endl;
        }
    }
    else if (cmd == "incp") {
        std::string hostPath, fsName;
        if (ss >> hostPath >> fsName) {
            if (ss >> extra) std::cout << "Error: Too many arguments. Usage: incp <host_path> <fs_name>" << std::endl;
            else copyFileFromHost(hostPath, fsName);
        } else {
            std::cout << "Usage: incp <host_path> <fs_name>" << std::endl;
        }
    }
    else if (cmd == "outcp") {
        std::string fsName, hostPath;
        if (ss >> fsName >> hostPath) {
            if (ss >> extra) std::cout << "Error: Too many arguments. Usage: outcp <fs_name> <host_path>" << std::endl;
            else copyFileToHost(fsName, hostPath);
        } else {
            std::cout << "Usage: outcp <fs_name> <host_path>" << std::endl;
        }
    }
    else if (cmd == "write") {
        std::string fileName;
        if (ss >> fileName) {
            std::string content;
            std::getline(ss, content);
            
            // Trim leading whitespace caused by the space after filename
            size_t firstChar = content.find_first_not_of(" \t");
            if (firstChar != std::string::npos) {
                content = content.substr(firstChar);
                writeFile(fileName, content);
            } else {
                // It is techincally valid to write empty content
                writeFile(fileName, "");
            }
        } else {
            std::cout << "Usage: write <filename> <text content>" << std::endl;
        }
    }
    else if (cmd == "xcp") {
        std::string s1, s2, s3;
        if (ss >> s1 >> s2 >> s3) {
            if (ss >> extra) {
                std::cout << "Error: Too many arguments. Usage: xcp <src1> <src2> <dest>" << std::endl;
            } else {
                extendedCopy(s1, s2, s3);
            }
        } else {
            std::cout << "Usage: xcp <src1> <src2> <dest>" << std::endl;
        }
    }
    else if (cmd == "add") {
        std::string src, dest;
        if (ss >> src >> dest) {
            if (ss >> extra) {
                std::cout << "Error: Too many arguments. Usage: add <src> <dest>" << std::endl;
            } else {
                addContent(src, dest);
            }
        } else {
            std::cout << "Usage: add <source> <destination>" << std::endl;
        }
    }
    else if (cmd == "pwd") {
        if (ss >> extra) {
            std::cout << "Error: 'pwd' takes no arguments." << std::endl;
        }
        else {
            printWorkingDirectory();
        }
    }
    else if (cmd == "load") {
        std::string scriptPath;
        if (ss >> scriptPath) {
            if (ss >> extra) std::cout << "Error: Too many args. Usage: load <script_path>" << std::endl;
            else runScript(scriptPath);
        } else {
            std::cout << "Usage: load <script_path>" << std::endl;
        }
    }
    else {
        std::cout << "Command '" << cmd << "' not recognized." << std::endl;
    }
    return true;
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
    std::memset(dot.name, 0, 12);
    std::strncpy(dot.name, ".", 11);
    dot.inodeNumber = newInodeId;

    DirEntry dotdot;
    std::memset(dotdot.name, 0, 12);
    std::strncpy(dotdot.name, "..", 11);
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
    std::memset(newEntry.name, 0, 12); 
    std::strncpy(newEntry.name, name.c_str(), 11); 
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

    std::string content = getFileContent(inodeId);
    
    if (content.empty()) { 
        return;
    }

    std::cout << content << std::endl;
}

void moveFile(const std::string& oldRaw, const std::string& newRaw) {
    // Sanitize both names
    std::string oldName = deleteTrailingWhitespace(oldRaw);

    std::string newName = deleteTrailingWhitespace(newRaw);

    if (!isValid83(newName)) {
        std::cerr << "Error: New name '" << newName << "' is too long (Max 11 chars)." << std::endl;
        return;
    }

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
                std::memset(entry->name, 0, 12);
                std::strncpy(entry->name, newName.c_str(), 11);
                
                // Write back
                file.seekp(blockOffset, std::ios::beg);
                file.write(block.data(), BLOCK_SIZE);
                
                std::cout << "Renamed '" << oldName << "' to '" << newName << "'." << std::endl;
                return;
            }
        }
    }
}

void copyFileFromHost(const std::string& hostPath, const std::string& fsPath) {
    std::ifstream hostFile(hostPath, std::ios::binary);
    if (!hostFile) {
        std::cerr << "Error: Host file '" << hostPath << "' not found." << std::endl;
        return;
    }

    std::string content((std::istreambuf_iterator<char>(hostFile)), std::istreambuf_iterator<char>());
    
    std::pair<int, std::string> dest = resolveParentAndName(fsPath);
    if (dest.first == -1) {
        std::cerr << "Error: Invalid destination path." << std::endl;
        return;
    }
    
    if (!isValid83(dest.second)) {
        std::cerr << "Error: Name '" << dest.second << "' is not 8.3 format." << std::endl;
        return;
    }

    // We temporarily switch to the destination directory to use our existing functions
    int savedInode = currentDirectoryInode;
    currentDirectoryInode = dest.first;
    
    createFile(dest.second);
    writeFile(dest.second, content);
    
    currentDirectoryInode = savedInode;
    std::cout << "Imported '" << hostPath << "' to '" << fsPath << "' (" << content.size() << " bytes)." << std::endl;
}

void copyFileToHost(const std::string& fsPath, const std::string& hostPath) {
    int inodeId = resolvePath(fsPath);
    if (inodeId == -1) {
        std::cerr << "Error: File '" << fsPath << "' not found in filesystem." << std::endl;
        return;
    }

    std::string content = getFileContent(inodeId);

    std::ofstream hostFile(hostPath, std::ios::binary);
    if (!hostFile) {
        std::cerr << "Error: Could not create host file '" << hostPath << "'." << std::endl;
        return;
    }
    hostFile.write(content.data(), content.size());
    std::cout << "Exported '" << fsPath << "' to '" << hostPath << "'." << std::endl;
}

std::string getFileContent(int inodeId) {
    std::string diskName = getDiskName();
    std::fstream file(diskName, std::ios::in | std::ios::binary);
    
    if (!file.is_open()) {
        std::cerr << "Error: Could not open disk." << std::endl;
        return "";
    }

    Inode inode;
    file.seekg(INODE_TABLE_OFFSET + (inodeId * sizeof(Inode)), std::ios::beg);
    file.read(reinterpret_cast<char*>(&inode), sizeof(Inode));

    if (inode.isDirectory) {
        std::cerr << "Error: '" << inodeId << "' is a directory." << std::endl;
        return "";
    }

    std::string content;
    content.reserve(inode.size);

    int bytesRemaining = inode.size;

    for (int i = 0; i < 10; ++i) {
        if (inode.directBlocks[i] == -1 || bytesRemaining <= 0) break;

        std::vector<char> buffer(BLOCK_SIZE);
        file.seekg(DATA_OFFSET + (inode.directBlocks[i] * BLOCK_SIZE), std::ios::beg);
        file.read(buffer.data(), BLOCK_SIZE);

        int bytesToRead = std::min(BLOCK_SIZE, bytesRemaining);
        content.append(buffer.data(), bytesToRead);
        
        bytesRemaining -= bytesToRead;
    }

    return content;
}

void showFileStats(const std::string& fileName) {
    int inodeId = findInodeId(fileName, currentDirectoryInode);
    if (inodeId == -1) {
        std::cerr << "Error: File '" << fileName << "' not found." << std::endl;
        return;
    }

    std::string diskName = getDiskName();
    std::fstream file(diskName, std::ios::in | std::ios::binary);
    
    // Load Inode
    Inode inode;
    file.seekg(INODE_TABLE_OFFSET + (inodeId * sizeof(Inode)), std::ios::beg);
    file.read(reinterpret_cast<char*>(&inode), sizeof(Inode));

    std::cout << "========= File Stats: " << fileName << " =========" << std::endl;
    std::cout << " Inode ID:      " << inodeId << std::endl;
    std::cout << " Type:          " << (inode.isDirectory ? "Directory" : "File") << std::endl;
    std::cout << " Size:          " << inode.size << " bytes" << std::endl;
    
    // Format Time
    char timeBuffer[80];
    struct tm* timeInfo = localtime(&inode.modifiedAt);
    strftime(timeBuffer, sizeof(timeBuffer), "%Y-%m-%d %H:%M:%S", timeInfo);
    std::cout << " Last Modified: " << timeBuffer << std::endl;

    std::cout << " -- Block Usage --" << std::endl;
    int blockCount = 0;
    std::cout << " Blocks Used:   [ ";
    for (int i = 0; i < 10; ++i) {
        if (inode.directBlocks[i] != -1) {
            std::cout << inode.directBlocks[i] << " ";
            blockCount++;
        }
    }
    std::cout << "]" << std::endl;
    std::cout << " Total Blocks:  " << blockCount << std::endl;
    std::cout << "==========================================" << std::endl;
}

void showDiskInfo() {
    std::string diskName = getDiskName();
    std::fstream file(diskName, std::ios::in | std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open disk." << std::endl;
        return;
    }

    // Read Superblock (Block 0)
    Superblock sb;
    file.seekg(0, std::ios::beg);
    file.read(reinterpret_cast<char*>(&sb), sizeof(Superblock));

    // Calculate percentages
    float usedBlocks = sb.totalBlocks - sb.freeBlocks;
    float blockUsage = (usedBlocks / sb.totalBlocks) * 100.0f;

    float usedInodes = sb.totalInodes - sb.freeInodes;
    float inodeUsage = (usedInodes / sb.totalInodes) * 100.0f;

    std::cout << "========= Disk Information =========" << std::endl;
    std::cout << " Disk Name:      " << diskName << std::endl;
    std::cout << " Total Size:     " << (sb.totalBlocks * BLOCK_SIZE) / 1024 << " KB" << std::endl;
    std::cout << " Block Size:     " << BLOCK_SIZE << " bytes" << std::endl;
    std::cout << "------------------------------------" << std::endl;
    std::cout << " Inodes:         " << sb.freeInodes << " free / " << sb.totalInodes << " total (" << usedInodes << " used)" << std::endl;
    std::cout << " Inode Usage:    " << inodeUsage << "%" << std::endl;
    std::cout << "------------------------------------" << std::endl;
    std::cout << " Data Blocks:    " << sb.freeBlocks << " free / " << sb.totalBlocks << " total (" << usedBlocks << " used)" << std::endl;
    std::cout << " Space Usage:    " << blockUsage << "%" << std::endl;
    std::cout << "====================================" << std::endl;
}

void extendedCopy(const std::string& src1, const std::string& src2, const std::string& dest) {
    // 1. Check Sources
    int inode1 = findInodeId(src1, currentDirectoryInode);
    if (inode1 == -1) {
        std::cerr << "Error: Source file '" << src1 << "' not found." << std::endl;
        return;
    }

    int inode2 = findInodeId(src2, currentDirectoryInode);
    if (inode2 == -1) {
        std::cerr << "Error: Source file '" << src2 << "' not found." << std::endl;
        return;
    }

    // 2. Check Destination
    if (findInodeId(dest, currentDirectoryInode) != -1) {
        std::cerr << "Error: Destination '" << dest << "' already exists." << std::endl;
        return;
    }

    // 3. Read Content (Load both into RAM)
    // We use the helper here because merging blocks manually is very complex
    std::string content1 = getFileContent(inode1);
    std::string content2 = getFileContent(inode2);

    // 4. Concatenate
    std::string combined = content1 + content2;

    // 5. Create and Write
    createFile(dest);
    writeFile(dest, combined);

    std::cout << "Created '" << dest << "' from '" << src1 << "' + '" << src2 << "'." << std::endl;
}

void addContent(const std::string& src, const std::string& dest) {
    int srcInode = findInodeId(src, currentDirectoryInode);
    if (srcInode == -1) {
        std::cerr << "Error: Source '" << src << "' not found." << std::endl;
        return;
    }

    int destInode = findInodeId(dest, currentDirectoryInode);
    if (destInode == -1) {
        std::cerr << "Error: Destination '" << dest << "' not found." << std::endl;
        return;
    }

    std::string srcContent = getFileContent(srcInode);
    std::string destContent = getFileContent(destInode);

    if (srcContent.size() + destContent.size() > 40960) { // 10 blocks * 4096 bytes
        std::cerr << "Error: Resulting file would be too large (Max 40KB)." << std::endl;
        return;
    }

    std::string combined = destContent + srcContent;

    writeFile(dest, combined);

    std::cout << "Appended content of '" << src << "' to '" << dest << "'." << std::endl;
}

void printWorkingDirectory() {
    if (currentDirectoryInode == 0) {
        std::cout << "/" << std::endl;
        return;
    }

    std::string fullPath = "";
    int curr = currentDirectoryInode;
    std::string diskName = getDiskName();

    while (curr != 0) {
        std::fstream file(diskName, std::ios::in | std::ios::binary);
        
        // 1. Get Parent Inode ID from ".."
        // ".." is always the 2nd entry (index 1) in the directory data
        Inode inode;
        file.seekg(INODE_TABLE_OFFSET + (curr * sizeof(Inode)), std::ios::beg);
        file.read(reinterpret_cast<char*>(&inode), sizeof(Inode));

        std::vector<char> block(BLOCK_SIZE);
        file.seekg(DATA_OFFSET + (inode.directBlocks[0] * BLOCK_SIZE), std::ios::beg);
        file.read(block.data(), BLOCK_SIZE);

        DirEntry* dotdot = reinterpret_cast<DirEntry*>(block.data() + sizeof(DirEntry)); // 2nd entry
        int parentId = dotdot->inodeNumber;

        // 2. Search PARENT to find what 'curr' is named
        Inode parentInode;
        file.seekg(INODE_TABLE_OFFSET + (parentId * sizeof(Inode)), std::ios::beg);
        file.read(reinterpret_cast<char*>(&parentInode), sizeof(Inode));
        
        std::vector<char> parentBlock(BLOCK_SIZE);
        file.seekg(DATA_OFFSET + (parentInode.directBlocks[0] * BLOCK_SIZE), std::ios::beg);
        file.read(parentBlock.data(), BLOCK_SIZE);

        std::string myName = "???";
        int maxEntries = BLOCK_SIZE / sizeof(DirEntry);
        for (int i = 0; i < maxEntries; ++i) {
            DirEntry* entry = reinterpret_cast<DirEntry*>(parentBlock.data() + (i * sizeof(DirEntry)));
            if (entry->inodeNumber == curr) {
                myName = entry->name;
                break;
            }
        }

        fullPath = "/" + myName + fullPath;
        curr = parentId;
    }

    std::cout << fullPath << std::endl;
}

void runScript(const std::string& hostPath) {
    std::ifstream scriptFile(hostPath);
    if (!scriptFile.is_open()) {
        std::cerr << "Error: Cannot open script '" << hostPath << "'" << std::endl;
        return;
    }

    std::string line;
    while (std::getline(scriptFile, line)) {
        if (line.empty() || line[0] == '#') continue;
        if (line.back() == '\r') line.pop_back();

        std::cout << "[Script] Executing: " << line << std::endl;     
        if (!executeCommand(line)) break;
    }
}
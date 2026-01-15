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
                currentDirectoryInode = 0;
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
        return;
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

    if (!inode.isDirectory) {
        std::cout << "Inode " << inodeId << " is not a directory." << std::endl;
        return;
    }
    
    // Calculate entries based on size, or just scan non-empty slots
    std::cout << "Listing Directory (Inode " << inodeId << "):" << std::endl;
    
    int maxEntries = BLOCK_SIZE / sizeof(DirEntry);
    for (int i = 0; i < maxEntries; ++i) {
        DirEntry* entry = reinterpret_cast<DirEntry*>(block.data() + (i * sizeof(DirEntry)));
        if (entry->inodeNumber != 0 || entry->name[0] != '\0') {
             std::cout << "  [" << entry->inodeNumber << "] " << entry->name << std::endl;
        }
    }
}

bool removeDirectory(const std::string& path) {
    std::pair<int, std::string> result = resolveParentAndName(path);
    int parentInodeId = result.first;
    std::string name = result.second;

    if (parentInodeId == -1) {
        std::cerr << "Error: Directory path not found." << std::endl;
        return false;
    }

    if (name == "." || name == "..") {
        std::cerr << "Error: Cannot delete system entries." << std::endl;
        return false;
    }

    std::string diskName = getDiskName();
    std::fstream file(diskName, std::ios::in | std::ios::out | std::ios::binary);
    if (!file.is_open()) return false;

    Inode parentInode;
    file.seekg(INODE_TABLE_OFFSET + (parentInodeId * sizeof(Inode)), std::ios::beg);
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
            // Check only valid entries
            if (std::string(entry->name) == name) {
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

    // --- SAFETY CHECK: Prevent deleting current directory ---
    if (targetInodeId == currentDirectoryInode) {
        std::cerr << "Error: Cannot remove current working directory." << std::endl;
        return false;
    }

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

    parentInode.size -= sizeof(DirEntry);
    file.seekp(INODE_TABLE_OFFSET + (parentInodeId * sizeof(Inode)), std::ios::beg);
    file.write(reinterpret_cast<char*>(&parentInode), sizeof(Inode));

    std::cout << "Directory '" << name << "' removed." << std::endl;
    return true;
}

void createFile(const std::string& path) {
    std::pair<int, std::string> result = resolveParentAndName(path);
    int parentInodeId = result.first;
    std::string fileName = result.second;

    if (fileName.empty() || !isValid83(fileName)) {
        std::cerr << "Error: Invalid file name." << std::endl;
        return;
    }

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
        return;
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

bool deleteFile(const std::string& path) {
    std::pair<int, std::string> result = resolveParentAndName(path);
    int parentInodeId = result.first;
    std::string name = result.second;

    if (parentInodeId == -1) {
        std::cerr << "Error: Directory path not found." << std::endl;
        return false;
    }

    if (name == "." || name == "..") {
        std::cerr << "Error: Cannot delete system entries." << std::endl;
        return false;
    }

    std::string diskName = getDiskName();
    std::fstream file(diskName, std::ios::in | std::ios::out | std::ios::binary);
    if (!file.is_open()) return false;

    Inode parentInode;
    int parentOffset = INODE_TABLE_OFFSET + (parentInodeId * sizeof(Inode));
    file.seekg(parentOffset, std::ios::beg);
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
            if (std::string(entry->name) == name) {
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
        std::cerr << "Error: '" << name << "' is a directory. Use rmdir." << std::endl;
        return false;
    }

    freeInodeBlocks(targetInode);
    freeInode(targetInodeId);

    std::memset(parentBlock.data() + (entryIndex * sizeof(DirEntry)), 0, sizeof(DirEntry));
    file.seekp(parentBlockOffset, std::ios::beg);
    file.write(parentBlock.data(), BLOCK_SIZE);

    parentInode.size -= sizeof(DirEntry);
    file.seekp(parentOffset, std::ios::beg);
    file.write(reinterpret_cast<char*>(&parentInode), sizeof(Inode));

    std::cout << "File '" << name << "' deleted." << std::endl;
    return true;
}

bool writeFile(const std::string& fileName, const std::string& content) {
    int inodeId = resolvePath(fileName);
    if (inodeId == -1) {
        std::cerr << "Error: File '" << fileName << "' not found. Create it first with 'touch'." << std::endl;
        return false;
    }

    long long contentSize = content.size();
    long long blocksNeeded = (contentSize + BLOCK_SIZE - 1) / BLOCK_SIZE;
    
    std::string diskName = getDiskName();
    std::fstream file(diskName, std::ios::in | std::ios::out | std::ios::binary);
    if (!file.is_open()) return false;

    Superblock sb;
    file.seekg(0, std::ios::beg);
    file.read(reinterpret_cast<char*>(&sb), sizeof(Superblock));

    long long estimatedOverhead = (blocksNeeded / 1024) + 2; 
    if (sb.freeBlocks < (blocksNeeded + estimatedOverhead)) {
        std::cerr << "Error: Not enough space." << std::endl;
        return false;
    }

    // Load Inode
    Inode inode;
    int inodeOffset = INODE_TABLE_OFFSET + (inodeId * sizeof(Inode));
    file.seekg(inodeOffset, std::ios::beg);
    file.read(reinterpret_cast<char*>(&inode), sizeof(Inode));

    if (inode.isDirectory) {
        std::cerr << "Error: Cannot write to a directory." << std::endl;
        return false;
    }

    freeInodeBlocks(inode);
    inode.size = 0;
    file.seekp(inodeOffset, std::ios::beg);
    file.write(reinterpret_cast<char*>(&inode), sizeof(Inode));

    int ptrsPerBlock = BLOCK_SIZE / sizeof(int);
    long long maxBlocks = 10 + ptrsPerBlock + (long long)ptrsPerBlock * ptrsPerBlock;
    
    if (blocksNeeded > maxBlocks) {
        std::cerr << "Error: File too large (Max ~4GB)." << std::endl;
        return false;
    }

    for (int i = 0; i < blocksNeeded; ++i) {
        int targetBlockId = -1;
        bool inodeUpdated = false;

        // Allocate block if not exists
        if (i < 10) {
            if (inode.directBlocks[i] == -1) {
                inode.directBlocks[i] = allocateDataBlock();
                if (inode.directBlocks[i] == -1) return false;
                inodeUpdated = true;
            }
            targetBlockId = inode.directBlocks[i];
        } 
        else if (i < 10 + ptrsPerBlock) {
            if (inode.singleIndirect == -1) {
                inode.singleIndirect = allocateDataBlock();
                if (inode.singleIndirect == -1) return false;
                std::vector<int> initPtrs(ptrsPerBlock, -1);
                file.seekp(DATA_OFFSET + (inode.singleIndirect * BLOCK_SIZE), std::ios::beg);
                file.write(reinterpret_cast<char*>(initPtrs.data()), BLOCK_SIZE);
            }

            std::vector<int> table(ptrsPerBlock);
            int tableOffset = DATA_OFFSET + (inode.singleIndirect * BLOCK_SIZE);
            file.seekg(tableOffset, std::ios::beg);
            file.read(reinterpret_cast<char*>(table.data()), BLOCK_SIZE);

            int idx = i - 10;
            if (table[idx] == -1) {
                table[idx] = allocateDataBlock();
                if (table[idx] == -1) return false;
                file.seekp(tableOffset, std::ios::beg);
                file.write(reinterpret_cast<char*>(table.data()), BLOCK_SIZE);
            }
            targetBlockId = table[idx];
        }
        else {
            // Allocate "Master" Double Indirect Block
            if (inode.doubleIndirect == -1) {
                inode.doubleIndirect = allocateDataBlock();
                if (inode.doubleIndirect == -1) return false;
                std::vector<int> initPtrs(ptrsPerBlock, -1);
                file.seekp(DATA_OFFSET + (inode.doubleIndirect * BLOCK_SIZE), std::ios::beg);
                file.write(reinterpret_cast<char*>(initPtrs.data()), BLOCK_SIZE);
            }

            long long relIndex = i - (10 + ptrsPerBlock);
            int outerIndex = relIndex / ptrsPerBlock; // Which Single Indirect block?
            int innerIndex = relIndex % ptrsPerBlock; // Which Data block inside that?

            // 3. Read Master Table
            std::vector<int> masterTable(ptrsPerBlock);
            int masterOffset = DATA_OFFSET + (inode.doubleIndirect * BLOCK_SIZE);
            file.seekg(masterOffset, std::ios::beg);
            file.read(reinterpret_cast<char*>(masterTable.data()), BLOCK_SIZE);

            // 4. Allocate the "Inner" Single Indirect Block if needed
            if (masterTable[outerIndex] == -1) {
                masterTable[outerIndex] = allocateDataBlock();
                if (masterTable[outerIndex] == -1) return false;
                
                // Initialize new Inner Table
                std::vector<int> initPtrs(ptrsPerBlock, -1);
                file.seekp(DATA_OFFSET + (masterTable[outerIndex] * BLOCK_SIZE), std::ios::beg);
                file.write(reinterpret_cast<char*>(initPtrs.data()), BLOCK_SIZE);

                // Save Master Table update
                file.seekp(masterOffset, std::ios::beg);
                file.write(reinterpret_cast<char*>(masterTable.data()), BLOCK_SIZE);
            }

            // 5. Read the Inner Table
            std::vector<int> innerTable(ptrsPerBlock);
            int innerOffset = DATA_OFFSET + (masterTable[outerIndex] * BLOCK_SIZE);
            file.seekg(innerOffset, std::ios::beg);
            file.read(reinterpret_cast<char*>(innerTable.data()), BLOCK_SIZE);

            // 6. Allocate the actual Data Block
            if (innerTable[innerIndex] == -1) {
                innerTable[innerIndex] = allocateDataBlock();
                if (innerTable[innerIndex] == -1) return false;
                
                // Save Inner Table update
                file.seekp(innerOffset, std::ios::beg);
                file.write(reinterpret_cast<char*>(innerTable.data()), BLOCK_SIZE);
            }
            targetBlockId = innerTable[innerIndex];
        }

        if (inodeUpdated) {
            file.seekp(inodeOffset, std::ios::beg);
            file.write(reinterpret_cast<char*>(&inode), sizeof(Inode));
        }


        long long pos = (long long)i * BLOCK_SIZE;
        long long len = std::min((long long)BLOCK_SIZE, contentSize - pos);
        
        std::string chunk = content.substr(pos, len);
        std::vector<char> buffer(BLOCK_SIZE, 0);
        std::memcpy(buffer.data(), chunk.c_str(), len);
        saveDataBlock(targetBlockId, buffer);
    }

    // Final Update
    inode.size = contentSize;
    inode.modifiedAt = time(nullptr);
    file.seekp(inodeOffset, std::ios::beg);
    file.write(reinterpret_cast<char*>(&inode), sizeof(Inode));

    return true;
}

bool copyFile(const std::string& srcRaw, const std::string& destRaw) {
    std::string srcPath = deleteTrailingWhitespace(srcRaw);
    std::string destPath = deleteTrailingWhitespace(destRaw);

    int srcInodeId = resolvePath(srcPath);
    if (srcInodeId == -1) {
        std::cerr << "Error: Source file '" << srcPath << "' not found." << std::endl;
        return false;
    }

    if (resolvePath(destPath) != -1) {
        std::cerr << "Error: Destination '" << destPath << "' already exists." << std::endl;
        return false;
    }

    std::string diskName = getDiskName();
    std::fstream file(diskName, std::ios::in | std::ios::out | std::ios::binary);
    if (!file.is_open()) return false;

    std::string content = getFileContent(srcInodeId);
    
    createFile(destPath);
    
    if (writeFile(destPath, content)) {
        return true;
    } 
    else {
        std::cerr << "Error: Copy failed. Deleting incomplete file." << std::endl;
        deleteFile(destPath); 
        return false;
    }
}

void readFile(const std::string& path) {
    int inodeId = resolvePath(path);
    if (inodeId == -1) {
        std::cerr << "Error: File '" << path << "' not found." << std::endl;
        return;
    }

    std::string content = getFileContent(inodeId);
    
    if (content.empty()) { 
        return;
    }

    std::cout << content << std::endl;
}

void moveFile(const std::string& srcRaw, const std::string& destRaw) {
    if (copyFile(srcRaw, destRaw)) {
    deleteFile(srcRaw);
    } else {
        std::cout << "Move failed. Source file preserved." << std::endl;
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
    long long bytesRemaining = inode.size;
    int ptrsPerBlock = BLOCK_SIZE / sizeof(int);

    long long totalBlocks = (inode.size + BLOCK_SIZE - 1) / BLOCK_SIZE;

    for (int i = 0; i < totalBlocks; ++i) {
        int blockToRead = -1;

        if (i < 10) {
            blockToRead = inode.directBlocks[i];
        } 
        else if (i < 10 + ptrsPerBlock) {
            if (inode.singleIndirect != -1) {
                std::vector<int> table(ptrsPerBlock);
                file.seekg(DATA_OFFSET + (inode.singleIndirect * BLOCK_SIZE), std::ios::beg);
                file.read(reinterpret_cast<char*>(table.data()), BLOCK_SIZE);
                blockToRead = table[i - 10];
            }
        } 
        else {
            if (inode.doubleIndirect != -1) {
                std::vector<int> masterTable(ptrsPerBlock);
                file.seekg(DATA_OFFSET + (inode.doubleIndirect * BLOCK_SIZE), std::ios::beg);
                file.read(reinterpret_cast<char*>(masterTable.data()), BLOCK_SIZE);

                long long relIndex = i - (10 + ptrsPerBlock);
                int outerIndex = relIndex / ptrsPerBlock;
                int innerIndex = relIndex % ptrsPerBlock;

                int innerTableBlock = masterTable[outerIndex];
                if (innerTableBlock != -1) {
                    std::vector<int> innerTable(ptrsPerBlock);
                    file.seekg(DATA_OFFSET + (innerTableBlock * BLOCK_SIZE), std::ios::beg);
                    file.read(reinterpret_cast<char*>(innerTable.data()), BLOCK_SIZE);
                    blockToRead = innerTable[innerIndex];
                }
            }
        }

        if (blockToRead != -1) {
            std::vector<char> buffer(BLOCK_SIZE);
            file.seekg(DATA_OFFSET + (blockToRead * BLOCK_SIZE), std::ios::beg);
            file.read(buffer.data(), BLOCK_SIZE);
            int bytesToRead = std::min((long long)BLOCK_SIZE, bytesRemaining);
            content.append(buffer.data(), bytesToRead);
            bytesRemaining -= bytesToRead;
        }
    }
    return content;
}

void showFileStats(const std::string& fileName) {
    int inodeId = resolvePath(fileName);
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

    int totalBlocks = blockCount;
    if (inode.singleIndirect != -1) {
        std::cout << " Indirect Block: " << inode.singleIndirect << " (Table)" << std::endl;
        totalBlocks++; // Count the table block itself

        // Load table to count actual data blocks used
        std::vector<int> table(BLOCK_SIZE / sizeof(int));
        file.seekg(DATA_OFFSET + (inode.singleIndirect * BLOCK_SIZE), std::ios::beg);
        file.read(reinterpret_cast<char*>(table.data()), BLOCK_SIZE);

        int indirectCount = 0;
        for (int blockId : table) {
            if (blockId != -1) {
                indirectCount++;
                totalBlocks++;
            }
        }
        std::cout << "   -> Holds " << indirectCount << " data blocks." << std::endl;
    }

    if (inode.doubleIndirect != -1) {
        std::cout << " Dbl Indirect:   " << inode.doubleIndirect << " (Master Table)" << std::endl;
        totalBlocks++; 

        std::vector<int> masterTable(BLOCK_SIZE / sizeof(int));
        file.seekg(DATA_OFFSET + (inode.doubleIndirect * BLOCK_SIZE), std::ios::beg);
        file.read(reinterpret_cast<char*>(masterTable.data()), BLOCK_SIZE);

        int innerTables = 0;
        int dataBlocks = 0;

        for (int innerBlockId : masterTable) {
            if (innerBlockId != -1) {
                innerTables++;
                totalBlocks++; // Count the inner table itself

                // Read Inner Table to count data blocks
                std::vector<int> innerTable(BLOCK_SIZE / sizeof(int));
                file.seekg(DATA_OFFSET + (innerBlockId * BLOCK_SIZE), std::ios::beg);
                file.read(reinterpret_cast<char*>(innerTable.data()), BLOCK_SIZE);

                for (int dataId : innerTable) {
                    if (dataId != -1) {
                        dataBlocks++;
                        totalBlocks++;
                    }
                }
            }
        }
        std::cout << "   -> Holds " << innerTables << " inner tables and " << dataBlocks << " data blocks." << std::endl;
    }

    std::cout << " Total Blocks:  " << totalBlocks << std::endl;
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
    int inode1 = resolvePath(src1);
    if (inode1 == -1) {
        std::cerr << "Error: Source file '" << src1 << "' not found." << std::endl;
        return;
    }

    int inode2 = resolvePath(src2);
    if (inode2 == -1) {
        std::cerr << "Error: Source file '" << src2 << "' not found." << std::endl;
        return;
    }

    // 2. Check Destination
    if (resolvePath(dest) != -1) {
        std::cerr << "Error: Destination '" << dest << "' already exists." << std::endl;
        return;
    }

    // We use the helper here because merging blocks manually is very complex
    std::string content1 = getFileContent(inode1);
    std::string content2 = getFileContent(inode2);

    std::string combined = content1 + content2;

    createFile(dest);
    writeFile(dest, combined);
}

void addContent(const std::string& src, const std::string& dest) {
    int srcInode = resolvePath(src);
    if (srcInode == -1) {
        std::cerr << "Error: Source '" << src << "' not found." << std::endl;
        return;
    }

    int destInode = resolvePath(dest);
    if (destInode == -1) {
        std::cerr << "Error: Destination '" << dest << "' not found." << std::endl;
        return;
    }

    std::string srcContent = getFileContent(srcInode);
    std::string destContent = getFileContent(destInode);

    std::string combined = destContent + srcContent;

    writeFile(dest, combined);
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
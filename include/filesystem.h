#include <string>
#include <iostream>

int startConsoleProgram();
int resolvePath(const std::string& path);
std::pair<int, std::string> resolveParentAndName(const std::string& path);
// File related functions
void createFile(const std::string& name);
bool deleteFile(const std::string& rawName);
bool writeFile(const std::string& fileName, const std::string& content);
void copyFile(const std::string& hostPath, const std::string& destFileName);
void readFile(const std::string& fileName);
void moveFile(const std::string& oldRaw, const std::string& newRaw);

// Directory related functions
bool addEntryToDirectory(int parentInodeId, const std::string& rawName, int newInodeId);
void listDirectory(int inodeId);
void createDirectory(const std::string& rawName);
void changeDirectory(const std::string& rawName);
bool removeDirectory(const std::string& rawName);
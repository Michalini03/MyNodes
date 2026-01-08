#include <string>
#include <iostream>

// Console program functions
int startConsoleProgram();
bool executeCommand(std::string commandLine);
void runScript(const std::string& hostPath);

// Path resolution functions
int resolvePath(const std::string& path);
std::pair<int, std::string> resolveParentAndName(const std::string& path);
void printWorkingDirectory();

// File related functions
void createFile(const std::string& name);
bool deleteFile(const std::string& rawName);
bool writeFile(const std::string& fileName, const std::string& content);
void copyFile(const std::string& hostPath, const std::string& destFileName);
void extendedCopy(const std::string& src1, const std::string& src2, const std::string& dest);
void readFile(const std::string& fileName);
void moveFile(const std::string& oldRaw, const std::string& newRaw);
void copyFileToHost(const std::string& fsPath, const std::string& hostPath);
void copyFileFromHost(const std::string& hostPath, const std::string& fsPath);
std::string getFileContent(int inodeId);
void addContent(const std::string& src, const std::string& dest);
void showFileStats(const std::string& fileName);

// Directory related functions
bool addEntryToDirectory(int parentInodeId, const std::string& rawName, int newInodeId);
void listDirectory(int inodeId);
void createDirectory(const std::string& rawName);
void changeDirectory(const std::string& rawName);
bool removeDirectory(const std::string& rawName);
void showDiskInfo();

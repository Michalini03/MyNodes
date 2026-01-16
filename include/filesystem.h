#include <string>
#include <iostream>

// --- Console program functions ---

/**
 * Starts the interactive console loop for the filesystem.
 * @return Exit code (0 for success).
 */
int startConsoleProgram();

/**
 * Parses and executes a single command line input.
 * @param commandLine The full command string (e.g., "mkdir /home/user").
 * @return True if the command executed successfully (or if the loop should continue), false to exit.
 */
bool executeCommand(std::string commandLine);

/**
 * Loads a text file from the host OS and executes every line as a command.
 * @param hostPath The path to the script file on the host computer.
 */
void runScript(const std::string& hostPath);

// --- Path resolution functions ---

/**
 * Resolves a path string to an Inode ID.
 * @param path The absolute or relative path (e.g., "/home/docs").
 * @return The Inode ID of the target, or -1 if not found.
 */
int resolvePath(const std::string& path);

/**
 * Splits a path into the parent directory's Inode ID and the target filename.
 * Used for creating or deleting files where you need the parent context.
 * @param path The full path to the target.
 * @return A pair containing {Parent Inode ID, Target Filename}.
 */
std::pair<int, std::string> resolveParentAndName(const std::string& path);

/**
 * Prints the current working directory path (absolute) to the console.
 */
void printWorkingDirectory();

// --- File related functions ---

/**
 * Creates a new empty file in the filesystem.
 * @param name The path/name of the file to create.
 */
void createFile(const std::string& name);

/**
 * Deletes a file from the filesystem and frees its blocks.
 * @param rawName The path/name of the file to delete.
 * @return True if deletion was successful.
 */
bool deleteFile(const std::string& rawName);

/**
 * Overwrites a file with specific content.
 * @param fileName The path to the file.
 * @param content The string content to write.
 * @return True if successful.
 */
bool writeFile(const std::string& fileName, const std::string& content);

/**
 * Copies a file within the filesystem.
 * @param srcPath The path of the source file.
 * @param destPath The path of the destination file.
 * @return True if successful.
 */
bool copyFile(const std::string& srcPath, const std::string& destPath);

/**
 * Merges two source files into a new destination file.
 * @param src1 Path to the first source file.
 * @param src2 Path to the second source file.
 * @param dest Path to the new destination file.
 */
void extendedCopy(const std::string& src1, const std::string& src2, const std::string& dest);

/**
 * Reads and prints the content of a file to the console.
 * @param fileName The path to the file.
 */
void readFile(const std::string& fileName);

/**
 * Moves or renames a file.
 * @param oldRaw The current path/name.
 * @param newRaw The new path/name.
 */
void moveFile(const std::string& oldRaw, const std::string& newRaw);

/**
 * Exports a file from the internal filesystem to the host OS.
 * @param fsPath Path of the file inside MyNodes.
 * @param hostPath Path where the file will be saved on the host computer.
 */
void copyFileToHost(const std::string& fsPath, const std::string& hostPath);

/**
 * Imports a file from the host OS into the internal filesystem.
 * @param hostPath Path of the file on the host computer.
 * @param fsPath Destination path inside MyNodes.
 */
void copyFileFromHost(const std::string& hostPath, const std::string& fsPath);

/**
 * Retrieves the full content of a file as a string.
 * @param inodeId The Inode ID of the file.
 * @return The content of the file.
 */
std::string getFileContent(int inodeId);

/**
 * Appends the content of the source file to the destination file.
 * @param src Path to the source file.
 * @param dest Path to the destination file (content will be appended here).
 */
void addContent(const std::string& src, const std::string& dest);

/**
 * Displays metadata (size, blocks used, permissions) for a file.
 * @param fileName Path to the file.
 */
void showFileStats(const std::string& fileName);

// --- Directory related functions ---

/**
 * Adds a directory entry (link) to a parent directory.
 * @param parentInodeId The ID of the directory to modify.
 * @param rawName The name of the new entry.
 * @param newInodeId The ID of the inode being linked.
 * @return True if successful.
 */
bool addEntryToDirectory(int parentInodeId, const std::string& rawName, int newInodeId);

/**
 * Lists the contents of a directory.
 * @param inodeId The Inode ID of the directory to list.
 */
void listDirectory(int inodeId);

/**
 * Creates a new directory.
 * @param rawName Path where the directory should be created.
 */
void createDirectory(const std::string& rawName);

/**
 * Changes the current working directory.
 * @param rawName Path to the target directory.
 */
void changeDirectory(const std::string& rawName);

/**
 * Removes an empty directory.
 * @param rawName Path to the directory.
 * @return True if successful.
 */
bool removeDirectory(const std::string& rawName);

/**
 * Displays overall disk usage statistics (free inodes, free blocks).
 */
void showDiskInfo();
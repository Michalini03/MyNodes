#include <iostream>
#include <vector>

/**
 * Removes whitespace characters from the end of a string.
 * @param rawName The input string.
 * @return Trimmed string.
 */
std::string deleteTrailingWhitespace(const std::string& rawName);

/**
 * Splits a path string by the '/' delimiter.
 * @param path The full path string (e.g., "/home/user/file").
 * @return A vector of path components (e.g., {"home", "user", "file"}).
 */
std::vector<std::string> splitPath(const std::string& path);

/**
 * Parses a size string with units into bytes.
 * @param sizeStr The string (e.g., "10MB", "512KB", "1024").
 * @return The size in bytes.
 */
long long parseSize(const std::string& sizeStr);

/**
 * Validates if a filename conforms to the 8.3 or 12-byte limit logic.
 * @param name The filename to check.
 * @return True if valid.
 */
bool isValid83(const std::string& name);
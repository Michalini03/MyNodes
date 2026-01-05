#include "../include/utils.h"
#include <sstream>

std::string deleteTrailingWhitespace(const std::string& rawName) {
    std::string name = rawName;
    size_t end = name.find_last_not_of(" \t\n\r");
    if (end != std::string::npos) {
        name = name.substr(0, end + 1);
    }
    return name;
}

std::vector<std::string> splitPath(const std::string& path) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(path);
    while (std::getline(tokenStream, token, '/')) {
        if (!token.empty()) {
            tokens.push_back(token);
        }
    }
    return tokens;
}
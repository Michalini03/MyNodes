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


bool isValid83(const std::string& name) {
    if (name.length() > 11) return false; 
    // . and .. are special and always valid
    if (name == "." || name == "..") return true;

    size_t dotPos = name.find('.');
    if (dotPos == std::string::npos) {
        return name.length() <= 8; 
    } else {
        // Has extension: Name <= 7, Dot = 1, Ext <= 3
        std::string base = name.substr(0, dotPos);
        std::string ext = name.substr(dotPos + 1);
        return (base.length() <= 8 && ext.length() <= 3);
    }
}

long long parseSize(const std::string& sizeStr) {
    std::string numberPart;
    std::string unitPart;
    
    for (char c : sizeStr) {
        if (isdigit(c)) numberPart += c;
        else unitPart += c;
    }
    
    if (numberPart.empty()) return 10 * 1024 * 1024;
    
    long long size = std::stoll(numberPart);
    
    // Check for M/MB or K/KB
    if (unitPart.find('M') != std::string::npos) size *= 1024 * 1024;
    else if (unitPart.find('K') != std::string::npos) size *= 1024;
    
    return size;
}


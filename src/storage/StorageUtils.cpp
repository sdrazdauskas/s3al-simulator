#include "storage/Storage.h"
#include <algorithm>
#include <iomanip>
#include <ctime>
#include <sstream>
#include <cctype>
#include <chrono>
#include <format>

namespace storage {

using Response = StorageManager::StorageResponse;

std::string StorageManager::toString(Response status) {
    switch (status) {
        case Response::OK:
            return "OK";
        case Response::AlreadyExists:
            return "Already Exists";
        case Response::NotFound:
            return "Not Found";
        case Response::AtRoot:
            return "Already at Root";
        case Response::InvalidArgument:
            return "Invalid Argument";
        case Response::Error:
        default:
            return "Error";
    }
}

bool StorageManager::isNameInvalid(const std::string& s) {
    if (s.empty()) return true;
    return std::all_of(s.begin(), s.end(), [](unsigned char c) {
        return std::isspace(c);
    });
}

void StorageManager::log(const std::string& level, const std::string& message) {
    if (logCallback) {
        logCallback(level, "STORAGE", message);
    }
}

std::string formatTime(const std::chrono::system_clock::time_point& tp) {
    auto seconds = std::chrono::floor<std::chrono::seconds>(tp);
    return std::format("{:%Y-%m-%d %H:%M:%S}", seconds);
}

StorageManager::PathInfo StorageManager::parsePath(const std::string& path) const {
    if (path.empty()) {
        return {nullptr, ""};
    }

    // is path absolute or relative
    bool isAbsolute = (path[0] == '/');
    Folder* current = isAbsolute ? root.get() : currentFolder;

    // split path by '/'
    std::vector<std::string> parts;
    std::string part;
    
    for (char c : path) {
        if (c == '/') {
            if (!part.empty()) {
                parts.push_back(part);
                part.clear();
            }
        } else {
            part += c;
        }
    }
    if (!part.empty()) {
        parts.push_back(part);
    }

    // handle "/" case, just return root with empty name
    if (parts.empty() && isAbsolute) {
        return {root.get(), ""};
    }

    if (parts.empty()) {
        return {nullptr, ""};
    }

    // navigate to parent folder
    for (size_t i = 0; i + 1 < parts.size(); ++i) {
        const std::string& dirName = parts[i];
        
        if (dirName == ".") {
            continue;
        } else if (dirName == "..") {
            if (current->parent) {
                current = current->parent;
            }
        } else {
            // find subfolder
            bool found = false;
            for (auto& sub : current->subfolders) {
                if (sub->name == dirName) {
                    current = sub.get();
                    found = true;
                    break;
                }
            }
            if (!found) {
                return {nullptr, ""};
            }
        }
    }

    // handle the last part
    const std::string& lastName = parts.back();
    
    // if last part is ".." or ".", handle specially
    if (lastName == "..") {
        if (current->parent) {
            return {current->parent, ""};
        }
        return {current, ""};
    }
    
    if (lastName == ".") {
        return {current, ""};
    }

    // return folder and filename/dirname
    return {current, parts.back()};
}

}  // namespace storage

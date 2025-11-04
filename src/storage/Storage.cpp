#include "Storage.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <iomanip>
#include <ctime>
#include <fstream>

namespace storage {

using Response = StorageManager::StorageResponse;
using json = nlohmann::json;

std::string StorageManager::toString(StorageResponse status) {
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

StorageManager::StorageManager() {
    root = std::make_unique<Folder>();
    root->name = "root";
    root->parent = nullptr;
    currentFolder = root.get();
}

void StorageManager::setLogCallback(LogCallback callback) {
    log_callback = callback;
}

void StorageManager::log(const std::string& level, const std::string& message) {
    if (log_callback) {
        log_callback(level, "STORAGE", message);
    }
}

int StorageManager::findFileIndex(const std::string& name) const {
    for (size_t i = 0; i < currentFolder->files.size(); ++i)
        if (currentFolder->files[i]->name == name)
            return static_cast<int>(i);
    return -1;
}

int StorageManager::findFolderIndex(const std::string& name) const {
    for (size_t i = 0; i < currentFolder->subfolders.size(); ++i)
        if (currentFolder->subfolders[i]->name == name)
            return static_cast<int>(i);
    return -1;
}

void StorageManager::recursiveDelete(Folder& folder) {
    folder.files.clear();
    for (auto& sub : folder.subfolders) recursiveDelete(*sub);
    folder.subfolders.clear();
}

Response StorageManager::fileExists(const std::string& name) const {
    if (isNameInvalid(name))
        return Response::InvalidArgument;

    return (findFileIndex(name) != -1)
               ? Response::OK
               : Response::NotFound;
}

Response StorageManager::createFile(const std::string& name) {
    if (isNameInvalid(name)) return Response::InvalidArgument;
    if (findFileIndex(name) != -1) {
        log("ERROR", "File already exists: " + name);
        return Response::AlreadyExists;
    }

    auto newFile = std::make_unique<File>();
    newFile->name = name;
    newFile->content = "";
    newFile->createdAt = std::chrono::system_clock::now();
    newFile->modifiedAt = std::chrono::system_clock::now();

    currentFolder->files.push_back(std::move(newFile));
    currentFolder->modifiedAt = std::chrono::system_clock::now();
    log("INFO", "Created file: " + name);
    return Response::OK;
}

Response StorageManager::touchFile(const std::string& name) {
    if (isNameInvalid(name)) return Response::InvalidArgument;
    int i = findFileIndex(name);

    if (i == -1) {
        log("INFO", "File does not exist, will be created: " + name);
        createFile(name);
        return Response::OK;
    }

    currentFolder->files[i]->modifiedAt = std::chrono::system_clock::now();
    currentFolder->modifiedAt = std::chrono::system_clock::now();
    log("INFO", "File already exists, timestamp was updated: " + name);
    return Response::OK;
}

Response StorageManager::deleteFile(const std::string& name) {
    if (isNameInvalid(name)) return Response::InvalidArgument;
    int i = findFileIndex(name);
    if (i == -1) {
        log("ERROR", "File not found: " + name);
        return Response::NotFound;
    }
    currentFolder->files.erase(currentFolder->files.begin() + i);
    currentFolder->modifiedAt = std::chrono::system_clock::now();
    log("INFO", "Deleted file: " + name);
    return Response::OK;
}

Response StorageManager::writeFile(const std::string& name,
                                   const std::string& content) {
    if (isNameInvalid(name)) return Response::InvalidArgument;
    int i = findFileIndex(name);
    if (i == -1) {
        log("ERROR", "File not found: " + name);
        return Response::NotFound;
    }
    currentFolder->files[i]->content = content + "\n";
    currentFolder->files[i]->modifiedAt = std::chrono::system_clock::now();
    currentFolder->modifiedAt = std::chrono::system_clock::now();
    log("INFO", "Wrote to file: " + name);
    return Response::OK;
}

Response StorageManager::readFile(const std::string& name,
                                  std::string& outContent) const {
    if (isNameInvalid(name)) return Response::InvalidArgument;
    int i = findFileIndex(name);
    if (i == -1) return Response::NotFound;
    outContent = currentFolder->files[i]->content;
    return Response::OK;
}

Response StorageManager::editFile(const std::string& name) {
    if (isNameInvalid(name)) return Response::InvalidArgument;
    int i = findFileIndex(name);
    if (i == -1) return Response::NotFound;

    File& f = *currentFolder->files[i];
    std::cout << "=== Editing " << name << " ===\n";
    if (f.content.empty()) std::cout << "(empty)\n";
    else std::cout << f.content;
    std::cout << "--------------------------------------\n";
    std::cout << "Type new content below to ADD to the file.\n";
    std::cout << "Type ':wq' on a new line to save and exit.\n";
    std::cout << "--------------------------------------\n";

    std::string newLines, line;
    while (true) {
        std::getline(std::cin, line);
        if (line == ":wq") break;
        newLines += line + "\n";
    }
    f.content += newLines;
    currentFolder->files[i]->modifiedAt = std::chrono::system_clock::now();
    currentFolder->modifiedAt = std::chrono::system_clock::now();
    return Response::OK;
}

Response StorageManager::makeDir(const std::string& name) {
    if (isNameInvalid(name)) return Response::InvalidArgument;
    if (findFolderIndex(name) != -1) {
        log("ERROR", "Directory already exists: " + name);
        return Response::AlreadyExists;
    }

    auto folder = std::make_unique<Folder>();
    folder->name = name;
    folder->parent = currentFolder;
    folder->createdAt = std::chrono::system_clock::now();
    folder->modifiedAt = std::chrono::system_clock::now();

    currentFolder->subfolders.push_back(std::move(folder));
    currentFolder->modifiedAt = std::chrono::system_clock::now();
    log("INFO", "Created directory: " + name);
    return Response::OK;
}

Response StorageManager::removeDir(const std::string& name) {
    if (isNameInvalid(name)) return Response::InvalidArgument;
    int i = findFolderIndex(name);
    if (i == -1) {
        log("ERROR", "Directory not found: " + name);
        return Response::NotFound;
    }
    currentFolder->subfolders.erase(currentFolder->subfolders.begin() + i);
    currentFolder->modifiedAt = std::chrono::system_clock::now();
    log("INFO", "Removed directory: " + name);
    return Response::OK;
}

Response StorageManager::changeDir(const std::string& path) {
    if (isNameInvalid(path)) return Response::InvalidArgument;
    if (path == "..") {
        if (currentFolder->parent == nullptr) return Response::AtRoot;
        currentFolder = currentFolder->parent;
        log("INFO", "Changed directory to: " + currentFolder->name);
        return Response::OK;
    }
    int i = findFolderIndex(path);
    if (i == -1) {
        log("ERROR", "Directory not found: " + path);
        return Response::NotFound;
    }
    currentFolder = currentFolder->subfolders[i].get();
    log("INFO", "Changed directory to: " + currentFolder->name);
    return Response::OK;
}

std::string formatTime(const std::chrono::system_clock::time_point& tp) {
    std::time_t time = std::chrono::system_clock::to_time_t(tp);
    std::tm tm = *std::localtime(&time);

    std::ostringstream ss;
    ss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

std::vector<std::string> StorageManager::listDir() const {
    std::vector<std::string> entries;

    for (auto& f : currentFolder->subfolders) {
        std::ostringstream line;
        line << "[D] " << f->name
             << " | created: " << formatTime(f->createdAt)
             << " | modified: " << formatTime(f->modifiedAt);
        entries.push_back(line.str());
    }

    for (auto& fl : currentFolder->files) {
        std::ostringstream line;
        line << "[F] " << fl->name
             << " | created: " << formatTime(fl->createdAt)
             << " | modified: " << formatTime(fl->modifiedAt);
        entries.push_back(line.str());
    }

    return entries;
}

std::string StorageManager::getWorkingDir() const {
    std::ostringstream path;
    std::vector<std::string> parts;
    auto* tmp = currentFolder;
    while (tmp) {
        parts.push_back(tmp->name);
        tmp = tmp->parent;
    }
    path << "/";
    for (int i = static_cast<int>(parts.size()) - 1; i >= 0; --i) {
        path << parts[i];
        if (i != 0) path << "/";
    }
    return path.str();
}

static json serializeFolder(const StorageManager::Folder& folder) {
    json j;
    j["name"] = folder.name;
    j["createdAt"] = std::chrono::duration_cast<std::chrono::seconds>(
                         folder.createdAt.time_since_epoch())
                         .count();
    j["modifiedAt"] = std::chrono::duration_cast<std::chrono::seconds>(
                          folder.modifiedAt.time_since_epoch())
                          .count();
    j["files"] = json::array();
    j["subfolders"] = json::array();

    for (auto& f : folder.files) {
        json jf;
        jf["name"] = f->name;
        jf["content"] = f->content;
        jf["createdAt"] = std::chrono::duration_cast<std::chrono::seconds>(
                              f->createdAt.time_since_epoch())
                              .count();
        jf["modifiedAt"] = std::chrono::duration_cast<std::chrono::seconds>(
                               f->modifiedAt.time_since_epoch())
                               .count();
        j["files"].push_back(jf);
    }

    for (auto& sub : folder.subfolders) {
        j["subfolders"].push_back(serializeFolder(*sub));
    }

    return j;
}

static std::unique_ptr<StorageManager::Folder> deserializeFolder(
    const json& j, StorageManager::Folder* parent) {
    auto folder = std::make_unique<StorageManager::Folder>();
    folder->name = j.at("name");
    folder->parent = parent;

    if (j.contains("createdAt")) {
        folder->createdAt = std::chrono::system_clock::time_point(
            std::chrono::seconds(j.at("createdAt").get<long long>()));
    } else {
        folder->createdAt = std::chrono::system_clock::now();
    }

    if (j.contains("modifiedAt")) {
        folder->modifiedAt = std::chrono::system_clock::time_point(
            std::chrono::seconds(j.at("modifiedAt").get<long long>()));
    } else {
        folder->modifiedAt = folder->createdAt;
    }

    if (j.contains("files")) {
        for (const auto& jf : j.at("files")) {
            auto f = std::make_unique<StorageManager::File>();
            f->name = jf.at("name");
            f->content = jf.at("content");

            if (jf.contains("createdAt"))
                f->createdAt = std::chrono::system_clock::time_point(
                    std::chrono::seconds(jf.at("createdAt").get<long long>()));
            else
                f->createdAt = std::chrono::system_clock::now();

            if (jf.contains("modifiedAt"))
                f->modifiedAt = std::chrono::system_clock::time_point(
                    std::chrono::seconds(jf.at("modifiedAt").get<long long>()));
            else
                f->modifiedAt = f->createdAt;

            folder->files.push_back(std::move(f));
        }
    }

    if (j.contains("subfolders")) {
        for (const auto& sub : j.at("subfolders")) {
            folder->subfolders.push_back(deserializeFolder(sub, folder.get()));
        }
    }

    return folder;
}

Response StorageManager::reset() {
    try {
        recursiveDelete(*root);
        root = std::make_unique<Folder>();
        root->name = "root";
        root->parent = nullptr;
        currentFolder = root.get();
        log("INFO", "Storage reset to empty state");
        return Response::OK;
    } catch (...) {
        return Response::Error;
    }
}

Response StorageManager::saveToDisk(const std::string& fileName) const {
    namespace fs = std::filesystem;
    try {
        fs::create_directories("data");

        std::string fullPath = "data/" + fileName;
        if (fullPath.find(".json") == std::string::npos)
            fullPath += ".json";

        json j = serializeFolder(*root);

        std::ofstream out(fullPath);
        out << std::setw(4) << j;
        out.close();
        return Response::OK;
    } catch (const std::exception& e) {
        return Response::Error;
    }
}

Response StorageManager::loadFromDisk(const std::string& fileName) {
    namespace fs = std::filesystem;
    try {
        std::string fullPath = "data/" + fileName;
        if (fullPath.find(".json") == std::string::npos)
            fullPath += ".json";

        if (!fs::exists(fullPath)) {
            log("ERROR", "File not found: " + fullPath);
            return Response::NotFound;
        }

        std::ifstream in(fullPath);
        json j;
        in >> j;
        root = deserializeFolder(j, nullptr);
        currentFolder = root.get();

        log("INFO", "Storage loaded from " + fullPath);
        return Response::OK;
    } catch (const std::exception& e) {
        log("ERROR", e.what());
        return Response::Error;
    }
}

} // namespace storage

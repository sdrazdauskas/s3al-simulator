#include "storage/Storage.h"
#include "kernel/SysCallsAPI.h"
#include <fstream>
#include <sstream>
#include <cstring>

namespace storage {

using json = nlohmann::json;
using Response = StorageManager::StorageResponse;

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
        if (f->memoryToken && f->contentSize > 0) {
            const char* src = static_cast<const char*>(f->memoryToken);
            jf["content"] = std::string(src, f->contentSize);
        } else {
            jf["content"] = "";
        }
        jf["createdAt"] = std::chrono::duration_cast<std::chrono::seconds>(
                              f->createdAt.time_since_epoch())
                              .count();
        jf["modifiedAt"] = std::chrono::duration_cast<std::chrono::seconds>(
                               f->modifiedAt.time_since_epoch())
                               .count();
        j["files"].push_back(jf);
    }

    for (auto& sub : folder.subfolders)
        j["subfolders"].push_back(serializeFolder(*sub));

    return j;
}

static std::unique_ptr<StorageManager::Folder> deserializeFolder(
    const json& j, StorageManager::Folder* parent, sys::SysApi* sysApi) {
    auto folder = std::make_unique<StorageManager::Folder>();
    folder->name = j.at("name");
    folder->parent = parent;

    folder->createdAt = std::chrono::system_clock::time_point(
        std::chrono::seconds(j.value("createdAt", 0LL)));
    folder->modifiedAt = std::chrono::system_clock::time_point(
        std::chrono::seconds(j.value("modifiedAt", 0LL)));

    for (const auto& jf : j["files"]) {
        auto f = std::make_unique<StorageManager::File>();
        f->name = jf.at("name");
        // Load content from JSON and allocate memory for it
        std::string content = jf.at("content");
        if (!content.empty() && sysApi) {
            f->memoryToken = sysApi->allocateMemory(content.size(), 0);
            if (f->memoryToken) {
                std::memcpy(f->memoryToken, content.c_str(), content.size());
                f->contentSize = content.size();
            } else {
                // Failed to allocate - file will have no content
                f->memoryToken = nullptr;
                f->contentSize = 0;
            }
        } else {
            f->memoryToken = nullptr;
            f->contentSize = 0;
        }
        f->createdAt = std::chrono::system_clock::time_point(
            std::chrono::seconds(jf.value("createdAt", 0LL)));
        f->modifiedAt = std::chrono::system_clock::time_point(
            std::chrono::seconds(jf.value("modifiedAt", 0LL)));
        folder->files.push_back(std::move(f));
    }

    for (const auto& sub : j["subfolders"])
        folder->subfolders.push_back(deserializeFolder(sub, folder.get(), sysApi));

    return folder;
}

Response StorageManager::saveToDisk(const std::string& fileName) const {
    try {
        std::filesystem::create_directories("data");
        std::string path = "data/" + fileName;
        if (path.find(".json") == std::string::npos) path += ".json";

        std::ofstream out(path);
        if (!out.is_open()) {
            return Response::Error;
        }
        out << std::setw(4) << serializeFolder(*root);
        if (!out) {
            return Response::Error;
        }
        return Response::OK;
    } catch (...) {
        return Response::Error;
    }
}

Response StorageManager::loadFromDisk(const std::string& fileName) {
    try {
        std::string fileNameWithExt = fileName;
        if (fileNameWithExt.find(".json") == std::string::npos) {
            fileNameWithExt += ".json";
        }

        std::string content;
        auto readResult = readFileFromHost(fileNameWithExt, content);
        if (readResult != Response::OK) {
            return readResult;
        }

        // Parse the JSON content
        json j = json::parse(content);
        root = deserializeFolder(j, nullptr, sysApi);
        currentFolder = root.get();
        return Response::OK;
    } catch (...) {
        return Response::Error;
    }
}

Response StorageManager::readFileFromHost(const std::string& hostFileName, std::string& outContent) {
    try {
        std::ifstream file;
        
        // Check if hostFileName is an absolute path
        std::filesystem::path filePath(hostFileName);
        if (filePath.is_absolute()) {
            // Use the path directly
            file.open(hostFileName);
        } else {
            // For relative paths, try in this order:
            // 1. Current directory / near executable
            file.open(hostFileName);
            
            if (!file.is_open()) {
                // 2. Try Docker/container path
                std::string dataPath = "/app/data/" + hostFileName;
                file.open(dataPath);
                
                if (!file.is_open()) {
                    // 3. Try local development data folder
                    dataPath = "data/" + hostFileName;
                    file.open(dataPath);
                }
            }
        }
        
        if (!file.is_open()) {
            return Response::NotFound;
        }
        
        // Read the entire file content
        std::stringstream buffer;
        buffer << file.rdbuf();
        outContent = buffer.str();
        file.close();
        
        if (outContent.empty()) {
            return Response::InvalidArgument;
        }
        
        return Response::OK;
    } catch (...) {
        return Response::Error;
    }
}

Response StorageManager::listDataFiles(std::vector<std::string>& outFiles) const {
    try {
        std::filesystem::path dataDir{"data"};
        if (!std::filesystem::exists(dataDir)) {
            return Response::NotFound;
        }

        outFiles.clear();

        for (auto& entry : std::filesystem::directory_iterator(dataDir)) {
            if (entry.is_regular_file() &&
                entry.path().extension() == ".json") {
                std::string fileName = entry.path().stem().string();
                outFiles.push_back(fileName);
            }
        }

        if (outFiles.empty()) return Response::NotFound;
        return Response::OK;
    } catch (...) {
        return Response::Error;
    }
}

}  // namespace storage

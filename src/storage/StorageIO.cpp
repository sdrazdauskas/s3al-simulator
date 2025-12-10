#include "storage/Storage.h"
#include <fstream>

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
        jf["content"] = f->content;
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
    const json& j, StorageManager::Folder* parent) {
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
        f->content = jf.at("content");
        f->createdAt = std::chrono::system_clock::time_point(
            std::chrono::seconds(jf.value("createdAt", 0LL)));
        f->modifiedAt = std::chrono::system_clock::time_point(
            std::chrono::seconds(jf.value("modifiedAt", 0LL)));
        folder->files.push_back(std::move(f));
    }

    for (const auto& sub : j["subfolders"])
        folder->subfolders.push_back(deserializeFolder(sub, folder.get()));

    return folder;
}

Response StorageManager::saveToDisk(const std::string& fileName) const {
    try {
        std::filesystem::create_directories("data");
        std::string path = "data/" + fileName;
        if (path.find(".json") == std::string::npos) path += ".json";

        std::ofstream out(path);
        out << std::setw(4) << serializeFolder(*root);
        return Response::OK;
    } catch (...) {
        return Response::Error;
    }
}

Response StorageManager::loadFromDisk(const std::string& fileName) {
    try {
        std::string path = "data/" + fileName;
        if (path.find(".json") == std::string::npos) path += ".json";

        if (!std::filesystem::exists(path)) {
            log("ERROR", "File not found: " + path);
            return Response::NotFound;
        }

        std::ifstream in(path);
        json j;
        in >> j;
        root = deserializeFolder(j, nullptr);
        currentFolder = root.get();
        log("INFO", "Storage loaded from: " + path);
        return Response::OK;
    } catch (const std::exception& e) {
        log("ERROR", e.what());
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

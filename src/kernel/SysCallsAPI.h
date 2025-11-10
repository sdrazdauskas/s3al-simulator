#pragma once
#include <string>
#include <vector>
 
namespace shell {

// TODO: error code system implementation?
enum class SysResult {
    OK,
    AlreadyExists,
    NotFound,
    AtRoot,
    InvalidArgument,
    Error
};

inline std::string toString(SysResult r) {
    switch(r) {
        case SysResult::OK: return "OK";
        case SysResult::AlreadyExists: return "AlreadyExists";
        case SysResult::NotFound: return "NotFound";
        case SysResult::AtRoot: return "AtRoot";
        case SysResult::InvalidArgument: return "InvalidArgument";
        default: return "Error";
    }
}

struct SysApi {
    struct SysInfo {
        size_t total_memory{0};
        size_t used_memory{0};
    };
    virtual SysResult readFile(const std::string& name, std::string& out) = 0;

    virtual SysResult createFile(const std::string& name) = 0;

    virtual SysResult deleteFile(const std::string& name) = 0;

    virtual SysResult writeFile(const std::string& name, const std::string& content) = 0;

    virtual SysResult editFile(const std::string& name, const std::string& newContent) = 0;

    virtual SysResult makeDir(const std::string& name) = 0;
    virtual SysResult removeDir(const std::string& name) = 0;
    virtual SysResult changeDir(const std::string& name) = 0;

    virtual SysResult saveToDisk(const std::string& fileName) = 0;
    virtual SysResult loadFromDisk(const std::string& fileName) = 0;
    virtual SysResult listDataFiles(std::vector<std::string>& out) = 0;
    virtual SysResult resetStorage() = 0;

    virtual SysResult copyFile(const std::string& src, const std::string& dest) = 0;
    virtual SysResult copyDir(const std::string& src, const std::string& dest) = 0;
    virtual SysResult moveFile(const std::string& src, const std::string& dest) = 0;
    virtual SysResult moveDir(const std::string& src, const std::string& dest) = 0;

    virtual std::vector<std::string> listDir() = 0;
    virtual std::string getWorkingDir() = 0;

    virtual SysInfo get_sysinfo() = 0;

    virtual SysResult fileExists(const std::string& name) = 0;


    virtual void requestShutdown() = 0;

    virtual ~SysApi() = default;
};

} // namespace shell

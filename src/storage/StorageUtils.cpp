#include "Storage.h"
#include <algorithm>
#include <iomanip>
#include <ctime>
#include <sstream>
#include <cctype>

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
    if (log_callback) {
        log_callback(level, "STORAGE", message);
    }
}

std::string formatTime(const std::chrono::system_clock::time_point& tp) {
    std::time_t time = std::chrono::system_clock::to_time_t(tp);
    std::tm tm = *std::localtime(&time);

    std::ostringstream ss;
    ss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

}  // namespace storage

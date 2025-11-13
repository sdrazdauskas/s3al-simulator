cmake_minimum_required(VERSION 3.22)
project(s3al LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find Threads package (required for std::thread)
find_package(Threads REQUIRED)

# Terminal library
add_library(terminal STATIC src/terminal/Terminal.cpp)
target_include_directories(terminal PUBLIC src/terminal)
target_link_libraries(terminal PUBLIC Threads::Threads)


# Shell library. command cmake maintained in src/shell/commands/CMakeLists.txt
add_subdirectory(src/shell/commands)
add_library(shell STATIC src/shell/Shell.cpp ${SHELL_COMMAND_SOURCES})
target_include_directories(shell PUBLIC src src/kernel src/shell)

# curses used by "texed" text editor command
find_package(Curses REQUIRED)
if (CURSES_FOUND)
    target_include_directories(shell PUBLIC ${CURSES_INCLUDE_DIR})
    target_link_libraries(shell PUBLIC ${CURSES_LIBRARIES})
endif()

# Config library (application-level configuration)
add_library(config STATIC src/config/Config.cpp)
target_include_directories(config PUBLIC src/config)

# Kernel library
add_library(kernel STATIC src/kernel/Kernel.cpp)
target_include_directories(kernel PUBLIC src/kernel src/shell src/terminal)
target_link_libraries(kernel PUBLIC terminal shell logging memory process storage scheduler Threads::Threads)

# Storage library
add_library(storage STATIC
    src/storage/Storage.cpp
    src/storage/StorageFileOps.cpp
    src/storage/StorageFolderOps.cpp
    src/storage/StorageIO.cpp
    src/storage/StorageUtils.cpp
)
target_include_directories(storage 
    PUBLIC src/storage
    PUBLIC ${CMAKE_SOURCE_DIR}/include
)

# Process library
add_library(process STATIC src/process/ProcessManager.cpp)
target_include_directories(process PUBLIC src/process)

# Memory library
add_library(memory STATIC src/memory/MemoryManager.cpp)
target_include_directories(memory PUBLIC src/memory)

# Scheduler library
add_library(scheduler STATIC src/scheduler/Scheduler.cpp)
target_include_directories(scheduler PUBLIC src/scheduler)

# Logging library
add_library(logging STATIC src/logger/Logger.cpp)
target_include_directories(logging PUBLIC src/logger)

# Main executable
add_executable(s3al_sim src/main.cpp)
target_link_libraries(s3al_sim PRIVATE config kernel logging)
target_include_directories(s3al_sim PRIVATE src)

# --- Unit Tests ---
include(FetchContent)


FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/heads/main.zip
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

enable_testing()

# Storage Tests
add_executable(storage_tests tests/storage_tests.cpp)
target_include_directories(storage_tests PRIVATE src/storage)
target_link_libraries(storage_tests PRIVATE storage GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(storage_tests)

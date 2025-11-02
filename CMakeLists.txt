cmake_minimum_required(VERSION 3.22)
project(s3al LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Terminal library
add_library(terminal STATIC src/terminal/Terminal.cpp)
target_include_directories(terminal PUBLIC src/terminal)

# Shell library
add_library(shell STATIC
        src/shell/Shell.cpp
        src/shell/commands/Cat.cpp
        src/shell/commands/Touch.cpp
        src/shell/commands/Echo.cpp
        src/shell/commands/Add.cpp
        src/shell/commands/Rm.cpp
        src/shell/commands/Write.cpp
        src/shell/commands/Edit.cpp
        src/shell/commands/Mkdir.cpp
        src/shell/commands/Rmdir.cpp
        src/shell/commands/Cd.cpp
        src/shell/commands/Ls.cpp
        src/shell/commands/Pwd.cpp
        src/shell/commands/Help.cpp
)

target_include_directories(shell PUBLIC
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/src/kernel
        ${CMAKE_SOURCE_DIR}/src/shell)

# Kernel library
add_library(kernel STATIC src/kernel/Kernel.cpp)
target_include_directories(kernel PUBLIC src/kernel src/shell src/terminal src/storage)
target_link_libraries(kernel PUBLIC terminal shell logging memory process storage scheduler)

# Storage library
add_library(storage STATIC src/storage/Storage.cpp)
target_include_directories(storage PUBLIC src/storage)

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
target_link_libraries(s3al_sim PRIVATE kernel logging)
target_include_directories(s3al_sim PRIVATE ${CMAKE_SOURCE_DIR}/src)

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
target_include_directories(storage_tests PRIVATE ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/src/storage)
target_link_libraries(storage_tests PRIVATE storage GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(storage_tests)

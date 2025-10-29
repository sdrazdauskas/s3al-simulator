cmake_minimum_required(VERSION 3.22)
project(s3al LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Terminal library
add_library(terminal STATIC src/terminal/Terminal.cpp)
target_include_directories(terminal PUBLIC src/terminal)

# Shell library
add_library(shell STATIC src/shell/Shell.cpp)
target_include_directories(shell PUBLIC src/shell)

# Storage library
add_library(storage STATIC src/storage/Storage.cpp)
target_include_directories(storage PUBLIC src/storage)

# Memory library
add_library(memory STATIC src/memory/MemoryManager.cpp)
target_include_directories(memory PUBLIC src/memory)

# Process library
add_library(process STATIC src/process/ProcessManager.cpp)
target_include_directories(process PUBLIC src/process src/memory src/scheduler)
target_link_libraries(process PUBLIC memory scheduler)

# Scheduler library
add_library(scheduler STATIC src/scheduler/scheduler.cpp)
target_include_directories(scheduler PUBLIC src/scheduler src/scheduler/sched_algos)

# Logger library
add_library(logger STATIC src/logger/Logger.cpp)
target_include_directories(logger PUBLIC src/logger)

# Kernel library
add_library(kernel STATIC src/kernel/Kernel.cpp)
target_include_directories(kernel PUBLIC src/kernel src/shell src/terminal src/storage src/scheduler src/process src/memory src/logger)
target_link_libraries(kernel PUBLIC terminal shell storage scheduler process memory logger)

# Main executable
add_executable(s3al_sim src/main.cpp)
target_link_libraries(s3al_sim PRIVATE kernel)

# --- Unit Tests ---
include(FetchContent)

FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/heads/main.zip
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

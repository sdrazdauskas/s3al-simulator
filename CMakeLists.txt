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

# Kernel library
add_library(kernel STATIC src/kernel/Kernel.cpp)
target_include_directories(kernel PUBLIC src/kernel src/shell src/terminal)
target_link_libraries(kernel PUBLIC terminal shell)

# Storage library
add_library(storage STATIC src/storage/Storage.cpp)
target_include_directories(storage PUBLIC src/storage)

# Process library
add_library(process STATIC src/process/ProcessManager.cpp)
target_include_directories(process PUBLIC src/process)

# Memory library
add_library(memory STATIC src/memory/Memory.cpp)
target_include_directories(memory PUBLIC src/memory)

# Main executable
add_executable(s3al_sim src/main.cpp)
target_link_libraries(s3al_sim PRIVATE kernel)


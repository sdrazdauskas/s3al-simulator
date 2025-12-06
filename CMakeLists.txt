cmake_minimum_required(VERSION 3.22)
project(s3al LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find Threads package (required for std::thread)
find_package(Threads REQUIRED)

# Terminal library
add_library(terminal STATIC)
target_sources(terminal PRIVATE
    src/terminal/Terminal.cpp
    src/terminal/helper/History.cpp
)
target_include_directories(terminal 
    PUBLIC src
    PRIVATE src/terminal
)
target_link_libraries(terminal PUBLIC Threads::Threads logging)

# Shell library. command cmake maintained in src/shell/commands/CMakeLists.txt
add_subdirectory(src/shell/commands)
add_library(shell STATIC)
target_sources(shell PRIVATE
    src/shell/Shell.cpp
    ${SHELL_COMMAND_SOURCES}
)
target_include_directories(shell 
    PUBLIC src
    PRIVATE src/shell
)

# curses used by "texed" text editor command
find_package(Curses REQUIRED)
target_include_directories(shell PUBLIC ${CURSES_INCLUDE_DIR})
target_link_libraries(shell PUBLIC ${CURSES_LIBRARIES})

# Config library (application-level configuration)
add_library(config STATIC)
target_sources(config PRIVATE src/config/Config.cpp)
target_include_directories(config 
    PUBLIC src
    PRIVATE src/config
)

# Kernel library
add_library(kernel STATIC)
target_sources(kernel PRIVATE src/kernel/Kernel.cpp)
target_include_directories(kernel 
    PUBLIC src
    PRIVATE src/kernel
)
target_link_libraries(kernel PUBLIC terminal shell logging memory process storage scheduler init Threads::Threads)

# Storage library
add_library(storage STATIC)
target_sources(storage PRIVATE
    src/storage/Storage.cpp
    src/storage/StorageFileOps.cpp
    src/storage/StorageFolderOps.cpp
    src/storage/StorageIO.cpp
    src/storage/StorageUtils.cpp
)
target_include_directories(storage 
    PUBLIC src ${CMAKE_SOURCE_DIR}/include
    PRIVATE src/storage
)

# Process library
add_library(process STATIC)
target_sources(process PRIVATE
    src/process/ProcessManager.cpp
    src/process/Process.cpp
)
target_include_directories(process 
    PUBLIC src
    PRIVATE src/process
)

# Daemon library (background services)
add_library(daemon STATIC)
target_sources(daemon PRIVATE
    src/daemon/Daemon.cpp
    src/daemon/MonitoringDaemon.cpp
    src/daemon/DaemonRegistry.cpp
)
target_include_directories(daemon 
    PUBLIC src
    PRIVATE src/daemon
)
target_link_libraries(daemon PUBLIC Threads::Threads)

# Init process (PID 1) - First user-space process
add_library(init STATIC)
target_sources(init PRIVATE src/init/Init.cpp)
target_include_directories(init 
    PUBLIC src ${CMAKE_SOURCE_DIR}/include
    PRIVATE src/init
)
target_link_libraries(init PUBLIC shell terminal daemon)

# Memory library
add_library(memory STATIC)
target_sources(memory PRIVATE src/memory/MemoryManager.cpp)
target_include_directories(memory 
    PUBLIC src
    PRIVATE src/memory
)

# Scheduler library
add_library(scheduler STATIC)
target_sources(scheduler PRIVATE src/scheduler/Scheduler.cpp)
target_include_directories(scheduler 
    PUBLIC src
    PRIVATE src/scheduler
)

# Logging library
add_library(logging STATIC)
target_sources(logging PRIVATE src/logger/Logger.cpp)
target_include_directories(logging 
    PUBLIC src
    PRIVATE src/logger
)

# Main executable
add_executable(s3al_sim)
target_sources(s3al_sim PRIVATE src/main.cpp)
target_include_directories(s3al_sim PRIVATE src)
target_link_libraries(s3al_sim PRIVATE config kernel logging)

# --- Unit Tests ---
include(FetchContent)

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

enable_testing()

# Storage Tests
add_executable(storage_tests)
target_sources(storage_tests PRIVATE tests/storage_tests.cpp)
target_link_libraries(storage_tests PRIVATE storage GTest::gtest_main GTest::gmock_main)

include(GoogleTest)
gtest_discover_tests(storage_tests)

# Process Manager Tests
add_executable(process_tests)
target_sources(process_tests PRIVATE tests/process_tests.cpp)
target_link_libraries(process_tests PRIVATE 
    process 
    memory 
    scheduler 
    logging
    GTest::gtest_main
    GTest::gmock_main
    Threads::Threads
)
gtest_discover_tests(process_tests)

# Shell Tests
add_executable(shell_tests)
target_sources(shell_tests PRIVATE tests/shell_tests.cpp)
target_include_directories(shell_tests PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(shell_tests PRIVATE 
    shell
    storage
    logging
    GTest::gtest_main
    GTest::gmock_main
)
gtest_discover_tests(shell_tests)

# Integration Tests
add_executable(integration_tests)
target_sources(integration_tests PRIVATE tests/integration_tests.cpp)
target_include_directories(integration_tests PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(integration_tests PRIVATE 
    kernel 
    storage 
    memory 
    process 
    scheduler 
    logging
    GTest::gtest_main
    GTest::gmock_main
    Threads::Threads
)
gtest_discover_tests(integration_tests)

# Acceptance Tests
add_executable(acceptance_tests)
target_sources(acceptance_tests PRIVATE tests/acceptance_tests.cpp)
target_include_directories(acceptance_tests PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(acceptance_tests PRIVATE 
    kernel 
    storage 
    memory 
    process 
    scheduler 
    logging
    GTest::gtest_main
    GTest::gmock_main
    Threads::Threads
)
gtest_discover_tests(acceptance_tests)

cmake_minimum_required(VERSION 3.22)
project(s3al LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(terminal STATIC
    src/terminal/Terminal.cpp
)

target_include_directories(terminal
    PUBLIC
        ${PROJECT_SOURCE_DIR}/src/terminal
)

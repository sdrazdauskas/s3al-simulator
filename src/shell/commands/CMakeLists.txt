## List of shell command sources maintained in this directory.
## Keeping the list here means adding a new command only requires editing
## files in this folder instead of touching the top-level CMakeLists.

set(SHELL_COMMAND_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/Cat.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Touch.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Echo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Add.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Rm.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Write.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Edit.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Mkdir.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Rmdir.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Cd.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Ls.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Pwd.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Help.cpp
    PARENT_SCOPE)

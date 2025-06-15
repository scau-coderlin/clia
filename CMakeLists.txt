cmake_minimum_required(VERSION 3.30)
project(clia)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

include_directories("inc")

aux_source_directory(src/log LogSrc)
aux_source_directory(src/util UtilSrc)
add_library(clia SHARED ${LogSrc} ${UtilSrc})

add_executable(logtest test/logtest.cpp)
target_link_libraries(logtest clia)

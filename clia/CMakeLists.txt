cmake_minimum_required(VERSION 3.20)
project(clia CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

aux_source_directory(src/log CLIA_LOG)
aux_source_directory(src/util CLIA_UTIL)
aux_source_directory(src/net CLIA_NET)
aux_source_directory(src/reactor CLIA_REACTOR)
add_library(clia OBJECT ${CLIA_LOG} ${CLIA_UTIL} ${CLIA_NET} ${CLIA_REACTOR})

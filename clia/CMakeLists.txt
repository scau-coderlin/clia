cmake_minimum_required(VERSION 3.30)
project(clia)

aux_source_directory(src/log CLIA_LOG)
aux_source_directory(src/util CLIA_UTIL)
add_library(clia SHARED ${CLIA_LOG} ${CLIA_UTIL})
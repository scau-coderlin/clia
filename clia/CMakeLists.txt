cmake_minimum_required(VERSION 3.20)
project(clia)

aux_source_directory(src/log CLIA_LOG)
aux_source_directory(src/util CLIA_UTIL)
aux_source_directory(src/net CLIA_NET)
aux_source_directory(src/reactor CLIA_REACTOR)
add_library(clia OBJECT ${CLIA_LOG} ${CLIA_UTIL} ${CLIA_NET} ${CLIA_REACTOR})

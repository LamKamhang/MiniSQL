cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project(minisql)

file(GLOB_RECURSE source_files ${PROJECT_SOURCE_DIR}/code/*.cpp)

add_executable(minisql
    ${source_files}
)
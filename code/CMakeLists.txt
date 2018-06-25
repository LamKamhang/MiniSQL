cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project(minisql)


add_executable(minisql
    Minisql.cpp
    API/API.cpp
    IndexManager/IndexManager.cpp
    BufferManager/BufferManager.cpp
    Interpreter/Interpreter.cpp
    RecordManager/RecordManager.cpp
    CatalogManager/CatalogManager.cpp
)
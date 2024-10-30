cmake_minimum_required(VERSION "3.5")

project(sym)

set(SOURCE_DIR ${CMAKE_SOURCE_DIR}/source)
file(GLOB_RECURSE SOURCES ${SOURCE_DIR}/*.cpp)

add_executable(sym ${SOURCES})
set_property(TARGET sym PROPERTY CXX_STANDARD 23)
target_include_directories(sym PRIVATE "source/")

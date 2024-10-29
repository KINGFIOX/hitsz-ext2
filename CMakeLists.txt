cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project(xv6fs VERSION 0.0.1 LANGUAGES C CXX)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_FILE_OFFSET_BITS=64 -no-pie -Wall -Werror -Wno-write-strings")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wall --pedantic -g")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_FILE_OFFSET_BITS=64 -no-pie -Wall -Werror -Wno-write-strings")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall --pedantic -g")

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake" ${CMAKE_MODULE_PATH})

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

find_package(FUSE REQUIRED)
include_directories(${FUSE_INCLUDE_DIR} ./inc ./inc/util)
add_executable("${PROJECT_NAME}" 
    "src/BlockCache.cxx"
    "src/fs.cxx"
    "src/INode.cxx"
    "src/INodeCache.cxx"
    "src/Log.cxx" 
    "src/main.cxx"
    "src/SuperBlock.cxx"
    "src/xv6fs.cxx"
    )
target_link_libraries("${PROJECT_NAME}" ${FUSE_LIBRARIES})

message("FUSE_INCLUDE_DIR ${FUSE_INCLUDE_DIR}")
message("FUSE_LIBRARIES ${FUSE_LIBRARIES}")
message("DIR_SRCS ${DIR_SRCS}")
message("!!!!!**CMAKE_GENERATOR** ${CMAKE_GENERATOR}")

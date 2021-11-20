cmake_minimum_required(VERSION 3.10)

set(CMAKE_INSTALL_PREFIX "Hayha")

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(include_path "${CMAKE_CURRENT_SOURCE_DIR}/inc")
set(include_path_api "${CMAKE_CURRENT_SOURCE_DIR}/inc/api")
set(include_path_ext "${CMAKE_CURRENT_SOURCE_DIR}/inc/external")
set(source_path "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(source_path_api "${CMAKE_CURRENT_SOURCE_DIR}/src/api")

include_directories(${include_path})
include_directories(${include_path_api})
include_directories(${include_path_ext})

file(GLOB SOURCES "${source_path}/*.cpp"  "${source_path_api}/*.cpp")


add_library(Happy STATIC ${SOURCES})

#target_link_libraries(Happy LINK_PRIVATE HCore)

target_include_directories(Happy PUBLIC ${include_path} ${include_path_api} ${include_path_ext})
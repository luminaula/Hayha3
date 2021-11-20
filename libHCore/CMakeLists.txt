cmake_minimum_required(VERSION 3.10)

set(include_path "${CMAKE_CURRENT_SOURCE_DIR}/inc")
set(source_path "${CMAKE_CURRENT_SOURCE_DIR}/src")

include_directories(${include_path})
file(GLOB SOURCES_COMMON "${source_path}/common/*.cpp")
if(UNIX)
    file(GLOB SOURCES_PLATFORM "${source_path}/linux/*.cpp")
else()
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS 1)
    file(GLOB SOURCES_PLATFORM "${source_path}/windows/*.cpp")
endif()



find_package (Threads)

add_library(HCore SHARED ${SOURCES_COMMON} ${SOURCES_PLATFORM})

target_include_directories(HCore PUBLIC ${include_path})

if(UNIX)
    install(TARGETS HCore LIBRARY DESTINATION libs/ COMPONENT Runtime)
else()
    install(TARGETS HCore RUNTIME DESTINATION bin/ COMPONENT Runtime)
endif()
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/inc)

file(GLOB SOURCES_COMMON "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

if(UNIX)
    file(GLOB SOURCES_PLATFORM
        "${CMAKE_CURRENT_SOURCE_DIR}/src/linux/*.cpp")
else()
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS 1)
    file(GLOB SOURCES_PLATFORM
        "${CMAKE_CURRENT_SOURCE_DIR}/src/windows/*.cpp")
endif()

find_package (Threads)

add_library(HInput SHARED ${SOURCES_COMMON} ${SOURCES_PLATFORM})

target_include_directories(HInput PUBLIC ${include_path})

target_link_libraries(HInput LINK_PUBLIC ${CMAKE_THREAD_LIBS_INIT})

if(UNIX)
    install(TARGETS HInput LIBRARY DESTINATION libs/ COMPONENT Runtime)
else()
    install(TARGETS HInput RUNTIME DESTINATION libs/ COMPONENT Runtime)
endif()
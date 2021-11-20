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
find_package(X11)

add_library(HCapture SHARED ${SOURCES_COMMON} ${SOURCES_PLATFORM})

target_include_directories(HCapture PUBLIC ${include_path})

target_link_libraries(HCapture LINK_PUBLIC ${CMAKE_THREAD_LIBS_INIT} ${X11_LIBRARIES} ${X11_Xext_LIB})

if(UNIX)
    install(TARGETS HCapture LIBRARY DESTINATION libs/ COMPONENT Runtime)
else()
    install(TARGETS HCapture RUNTIME DESTINATION libs/ COMPONENT Runtime)
endif()
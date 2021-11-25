cmake_minimum_required(VERSION 3.19)

set(CMAKE_INSTALL_PREFIX "Hayha")

if(UNIX)
    set(CMAKE_INSTALL_RPATH "$ORIGIN/../libs/")
else()
    set(CMAKE_ENABLE_EXECUTABLE_WRAPPER ON)
    set(CMAKE_INSTALL_RPATH "$ORIGIN\\..\\libs")
endif()
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

set(CMAKE_CXX_STANDARD 17)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/utils/inc)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/workers/inc)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/api/inc)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/image/inc)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/OS/inc/common)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/settings/inc)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/module/inc)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/gui/src)


file(GLOB SOURCES_DAEMON
    "${CMAKE_CURRENT_SOURCE_DIR}/program/Hayha-d/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/settings/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/OS/src/common/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/image/src/*.cpp"
)

file(GLOB SOURCES_APP 
    "${CMAKE_CURRENT_SOURCE_DIR}/gui/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/workers/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/api/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/image/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/module/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/OS/src/common/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/settings/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/program/Hayha-gui/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/utils/src/*.cpp"
)

file(GLOB SOURCES_TRAINER
    "${CMAKE_CURRENT_SOURCE_DIR}/program/Hayha-t/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/settings/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/OS/src/common/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/image/src/*.cpp")

if(UNIX)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/OS/inc/linux)
    file(GLOB SOURCES_APP_PLATFORM
        "${CMAKE_CURRENT_SOURCE_DIR}/OS/src/linux/*.cpp")
else()
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/OS/inc/windows)
    file(GLOB SOURCES_APP_PLATFORM
        "${CMAKE_CURRENT_SOURCE_DIR}/OS/src/windows/*.cpp")
endif()


find_package(Threads REQUIRED)
find_package(OpenGL REQUIRED)
find_package(X11)
find_package(Qt5 COMPONENTS Core Gui Widgets OpenGL Multimedia REQUIRED)


add_executable(Hayha ${SOURCES_APP_PLATFORM} ${SOURCES_APP})
add_executable(Hayha-d ${SOURCES_DAEMON} ${SOURCES_APP_PLATFORM})
add_executable(Hayha-t ${SOURCES_TRAINER} ${SOURCES_APP_PLATFORM})


if (UNIX)
    
    
    target_link_libraries(Hayha LINK_PUBLIC stdc++fs HCore Happy ${CMAKE_THREAD_LIBS_INIT} ${X11_LIBRARIES} ${X11_Xext_LIB} X11 Xext Qt5::Gui Qt5::Widgets Qt5::Core Qt5::OpenGL Qt5::Multimedia ${OPENGL_LIBRARIES} dl "${PROJECT_SOURCE_DIR}/linux/liblua53.a")
    target_link_libraries(Hayha-d LINK_PUBLIC stdc++fs HCore Happy ${X11_LIBRARIES} ${X11_Xext_LIB} X11 Xext dl Qt5::Widgets Qt5::Core)
    target_link_libraries(Hayha-t LINK_PUBLIC stdc++fs HCore Happy ${X11_LIBRARIES} ${X11_Xext_LIB} X11 Xext dl Qt5::Widgets Qt5::Core)
    include_directories(${X11_INCLUDE_DIR})
    install(DIRECTORY "${PROJECT_SOURCE_DIR}/linux/" DESTINATION libs/)   
else()
    
    set(CMAKE_VS_INCLUDE_INSTALL_TO_DEFAULT_BUILD 1)
    target_link_libraries(Hayha LINK_PUBLIC HCore Happy ${CMAKE_THREAD_LIBS_INIT} Qt5::Widgets Qt5::Core Qt5::OpenGL ${OPENGL_LIBRARIES} "${PROJECT_SOURCE_DIR}/windows/liblua53.a")
    target_link_libraries(Hayha-d LINK_PUBLIC HCore Happy Qt5::Widgets Qt5::Core)
    target_link_libraries(Hayha-t LINK_PUBLIC HCore Happy Qt5::Widgets Qt5::Core)
    install(DIRECTORY "${PROJECT_SOURCE_DIR}/windows/" DESTINATION bin/)
endif()



install(TARGETS Hayha Hayha-d Hayha-t RUNTIME DESTINATION bin/)


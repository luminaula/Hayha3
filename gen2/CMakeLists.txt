cmake_minimum_required(VERSION 3.10)

set(CMAKE_INSTALL_PREFIX "Hayha")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/")

if(UNIX)
    set(CMAKE_INSTALL_RPATH "$ORIGIN/../libs/")
else()
    set(CMAKE_ENABLE_EXECUTABLE_WRAPPER ON)
    set(CMAKE_INSTALL_RPATH "$ORIGIN\\..\\libs")
endif()
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

enable_language(CXX)
enable_language(C)

find_package(OpenCV REQUIRED)

find_package(Threads REQUIRED)

set(CMAKE_CXX_STANDARD 17)

file(GLOB SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
file(GLOB CROPPER "${CMAKE_CURRENT_SOURCE_DIR}/cropper/*.cpp")
file(GLOB BLENDER "${CMAKE_CURRENT_SOURCE_DIR}/blender/*.cpp")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/inc)

add_executable(gen ${SOURCES})
add_executable(cropper ${CROPPER})
add_executable(blender ${BLENDER})


target_link_libraries(gen assimp z m dl glut GL HCore ${OpenCV_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(cropper HCore ${OpenCV_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(blender HCore ${OpenCV_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
install(TARGETS gen cropper blender RUNTIME DESTINATION bin/)
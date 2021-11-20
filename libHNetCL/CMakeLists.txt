cmake_minimum_required(VERSION 3.10)

set(source_path "${CMAKE_CURRENT_SOURCE_DIR}/darknet_cl/")

find_package(OpenCL REQUIRED)

find_package(Threads REQUIRED)

file(GLOB SOURCES "${source_path}/src/*.cpp")
file(GLOB SOURCES_EX "${source_path}/example/*.cpp")

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS 1)


include_directories(./darknet_cl/include)
include_directories(./darknet_cl/clBLAS)
include_directories(${OpenCL_INCLUDE_DIR})


add_library(HNetCl STATIC ${SOURCES})
#add_executable(HNetCl-test ${SOURCES} ${SOURCES_EX})
#target_compile_definitions(HNetCl-test PUBLIC -DGPU)

target_compile_definitions(HNetCl PUBLIC -DGPU)



if(UNIX)
    add_library(clBlas SHARED IMPORTED)
    
    set_target_properties(clBlas PROPERTIES
        IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/darknet_cl/clBLAS/libclBLAS.so"
        IMPORTED_NO_SONAME TRUE)

    target_link_libraries(HNetCl ${CMAKE_THREAD_LIBS_INIT} ${OpenCL_LIBRARY} clBlas Happy)
    #target_link_libraries(HNetCl-test ${CMAKE_THREAD_LIBS_INIT} ${OpenCL_LIBRARY} clBlas Happy)
else()
    target_link_libraries(HNetCl ${CMAKE_THREAD_LIBS_INIT} ${OpenCL_LIBRARY} ${CMAKE_CURRENT_SOURCE_DIR}/darknet_cl/clBLAS/clBLAS.lib Happy)
    #target_link_libraries(HNetCl-test ${CMAKE_THREAD_LIBS_INIT} ${OpenCL_LIBRARY} ${CMAKE_CURRENT_SOURCE_DIR}/darknet_cl/clBLAS/clBLAS.lib Happy)
endif()


target_include_directories(HNetCl PUBLIC ./darknet_cl/include ./darknet_cl/clBLAS ${OpenCL_INCLUDE_DIR})
#
#if(UNIX)
#    install(TARGETS HNetCl LIBRARY DESTINATION libs/ COMPONENT Runtime)
#else()
#   install(TARGETS HNetCl RUNTIME DESTINATION libs/ COMPONENT Runtime)
#endif()
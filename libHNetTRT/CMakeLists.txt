set(CMAKE_INSTALL_PREFIX "Hayha")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/")



if(UNIX)
    set(TensorRT_ROOT "${PROJECT_SOURCE_DIR}/TensorRT-7.0.0.11/")
else()
    cmake_policy(SET CMP0074 NEW)
    set(TensorRT_ROOT "C:/TensorRT-5.0.4.3")
endif()
if(UNIX)
    set(CMAKE_INSTALL_RPATH "$ORIGIN/../libs/")
else()
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS 1)
endif()
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)


project(HNetTRT LANGUAGES C CXX CUDA)

set(CMAKE_VS_PLATFORM_TOOLSET_CUDA 10.2)
set(CMAKE_CUDA_FLAGS
    "-gencode arch=compute_70,code=[sm_70,compute_70] -gencode arch=compute_75,code=[sm_75,compute_75]"
)

find_package(TensorRT REQUIRED)


link_directories("/usr/local/cuda/lib64")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/inc/)
include_directories(${TensorRT_INCLUDE_DIRS})
include_directories(${OpenCV_INCLUDE_DIRS})

set(source_path "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(source_path_cuda "${CMAKE_CURRENT_SOURCE_DIR}/src_cuda")
set(test_path "${CMAKE_CURRENT_SOURCE_DIR}/test")
set(header_path "${CMAKE_CURRENT_SOURCE_DIR}/inc")
set(header_path_trt "${TENSORRT_PATH}/include")


file(GLOB SOURCES_COMMON 
    "${source_path}/*.c"  )
file(GLOB SOURCES_CPP
    "${source_path}/*.cpp")
file(GLOB SOURCES_CUDA
    "${source_path_cuda}/*.cu")
file(GLOB HEADERS
    "${header_path}/*.hpp"
    "${header_path}/*.h"
    "${header_path_trt}/*.h")

file(GLOB TESTS "${test_path}/*.cpp")





add_library(HNetTRT SHARED ${SOURCES_COMMON} ${SOURCES_CPP} ${SOURCES_CUDA} ${HEADERS})
#add_executable(Test ${TESTS} ${SOURCES_COMMON} ${SOURCES_CPP} ${SOURCES_CUDA} ${HEADERS})



#target_link_libraries(Test LINK_PUBLIC Happy HCore cublas curand cudnn ${TensorRT_LIBRARIES} "${TensorRT_ROOT}/lib/nvinfer_plugin.lib" ${OpenCV_LIBRARIES})


if(UNIX)
target_link_libraries(HNetTRT LINK_PUBLIC Happy HCore cublas curand cudnn ${TensorRT_LIBRARIES} "${TensorRT_ROOT}/lib/libnvinfer_plugin.so")
    install(TARGETS HNetTRT LIBRARY DESTINATION libs/ COMPONENT Runtime)
else()
    target_link_libraries(HNetTRT LINK_PUBLIC Happy HCore cublas curand cudnn ${TensorRT_LIBRARIES} "${TensorRT_ROOT}/lib/nvinfer_plugin.lib")
    install(TARGETS HNetTRT RUNTIME DESTINATION libs/ COMPONENT Runtime)
endif()
#install(TARGETS Test RUNTIME DESTINATION bin/)
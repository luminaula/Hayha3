cmake_minimum_required(VERSION 3.10)


set(CMAKE_INSTALL_PREFIX "Hayha")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/")


SET(ENV{LD_LIBRARY_PATH} "/usr/local/cuda/lib64:$ENV{LD_LIBRARY_PATH}")

project(HNet LANGUAGES C CXX CUDA)

set(CMAKE_VS_PLATFORM_TOOLSET_CUDA 10.0)
set(CMAKE_CUDA_FLAGS
    "  -gencode arch=compute_52,code=[sm_52,compute_52]  -gencode arch=compute_61,code=[sm_61,compute_61]  -gencode arch=compute_70,code=[sm_70,compute_70] -gencode arch=compute_75,code=[sm_75,compute_75]"
)

find_package(OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

find_package (Threads)


include_directories(${CMAKE_CURRENT_SOURCE_DIR}/inc/)

set(source_path "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(source_path_cuda "${CMAKE_CURRENT_SOURCE_DIR}/src_cuda")
set(header_path "${CMAKE_CURRENT_SOURCE_DIR}/inc")

file(GLOB SOURCES_COMMON 
    "${source_path}/*.c"  )
file(GLOB SOURCES_CPP
    "${source_path}/*.cpp")
file(GLOB SOURCES_CUDA
    "${source_path_cuda}/*.cu")
file(GLOB HEADERS
    "${header_path}/*.h")

#set(CUDALIBS "/usr/local/cuda/lib64/libcublas.so" "/usr/local/cuda/lib64/libcurand.so" "/usr/local/cuda/lib64/libcudnn.so")

link_directories("/usr/local/cuda/lib64")

add_library(HNetCUDA SHARED ${SOURCES_COMMON} ${SOURCES_CPP} ${SOURCES_CUDA} ${HEADERS})
#add_library(HNetCUDNN SHARED ${SOURCES_COMMON} ${SOURCES_CPP} ${SOURCES_CUDA} ${HEADERS})
#add_library(HNetCUDNNH SHARED ${SOURCES_COMMON} ${SOURCES_CPP} ${SOURCES_CUDA} ${HEADERS})
#add_library(HNetCPU SHARED ${SOURCES_COMMON} ${SOURCES_CPP})

target_compile_definitions(HNetCUDA PUBLIC -DGPU)
#target_compile_definitions(HNetCUDNN PUBLIC -DGPU -DCUDNN)
#target_compile_definitions(HNetCUDNNH PUBLIC -DGPU -DCUDNN -DCUDNN_HALF)

target_compile_features(HNetCUDA PUBLIC cxx_std_11)
#target_compile_features(HNetCUDNN PUBLIC cxx_std_11)
#target_compile_features(HNetCUDNNH PUBLIC cxx_std_11)
#target_compile_features(HNetCPU PUBLIC cxx_std_11)

set_target_properties(HNetCUDA PROPERTIES POSITION_INDEPENDENT_CODE ON)
set_target_properties(HNetCUDA PROPERTIES CUDA_SEPARABLE_COMPILATION ON)
#set_target_properties(HNetCUDNN PROPERTIES POSITION_INDEPENDENT_CODE ON)
#set_target_properties(HNetCUDNN PROPERTIES CUDA_SEPARABLE_COMPILATION ON)
#set_target_properties(HNetCUDNNH PROPERTIES POSITION_INDEPENDENT_CODE ON)
#set_target_properties(HNetCUDNNH PROPERTIES CUDA_SEPARABLE_COMPILATION ON)

target_link_libraries (HNetCUDA LINK_PUBLIC Happy HCore cublas curand ${CUDALIBS} ${CMAKE_THREAD_LIBS_INIT})
#target_link_libraries (HNetCUDNN LINK_PUBLIC Happy HCore cublas curand cudnn ${CUDALIBS} ${CMAKE_THREAD_LIBS_INIT})
#target_link_libraries (HNetCUDNNH LINK_PUBLIC Happy HCore cublas curand cudnn ${CUDALIBS} ${CMAKE_THREAD_LIBS_INIT})
#target_link_libraries (HNetCPU LINK_PUBLIC Happy HCore ${CMAKE_THREAD_LIBS_INIT})



if(UNIX)
    install(TARGETS HNetCUDA LIBRARY DESTINATION libs/ COMPONENT Runtime)
    #install(TARGETS HNetCUDNN LIBRARY DESTINATION libs/ COMPONENT Runtime)
    #install(TARGETS HNetCUDNNH LIBRARY DESTINATION libs/ COMPONENT Runtime)
    #install(TARGETS HNetCPU LIBRARY DESTINATION libs/ COMPONENT Runtime)
else()
    install(TARGETS HNetCUDA RUNTIME DESTINATION libs/ COMPONENT Runtime)
    #install(TARGETS HNetCUDNN RUNTIME DESTINATION libs/ COMPONENT Runtime)
    #install(TARGETS HNetCUDNNH RUNTIME DESTINATION libs/ COMPONENT Runtime)
    #install(TARGETS HNetCPU RUNTIME DESTINATION libs/ COMPONENT Runtime)
endif()

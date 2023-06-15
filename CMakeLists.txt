cmake_minimum_required(VERSION 3.10)

set(CMAKE_INSTALL_PREFIX "Hayha")

set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "~/Qt/5.11.1/gcc_64")

project(Hayha)

enable_language(CXX)
enable_language(C)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()


set(CMAKE_CXX_STANDARD 17)


if (UNIX)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -O0 -ggdb -ffp-contract=fast -mavx -msse4.1 -msse4a -I/usr/local/cuda/include")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ofast -ffp-contract=fast -mavx -msse4.1 -msse4a -I/usr/local/cuda/include")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wall -O0 -ggdb -ffp-contract=fast -mavx -msse4.1 -msse4a -I/usr/local/cuda/include")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Ofast -ffp-contract=fast -mavx -msse4.1 -msse4a -I/usr/local/cuda/include")
else()
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Ox /MP8")
endif()



add_subdirectory(libHappy)
add_subdirectory(libHCore)
# add_subdirectory(libHNet)


#add_subdirectory(libHNetCL)
add_subdirectory(libHInput)
add_subdirectory(App)

add_subdirectory(libHCapture)
# add_subdirectory(libHNetTRT)

if(UNIX)
    # add_subdirectory(gen2)
    #add_subdirectory(Generator)
    #add_subdirectory(libHDetector)
    # add_subdirectory(libHtensor)
    #add_subdirectory(libHTracker)
    # add_subdirectory(libHNetPy)
    
endif()
install(DIRECTORY "${PROJECT_SOURCE_DIR}/config/" DESTINATION cfg/)
install(DIRECTORY "${PROJECT_SOURCE_DIR}/profiles/" DESTINATION profiles/)
install(DIRECTORY "${PROJECT_SOURCE_DIR}/scripts/" DESTINATION scripts/)
install(DIRECTORY "${PROJECT_SOURCE_DIR}/tools/" DESTINATION tools/)
install(DIRECTORY "${PROJECT_SOURCE_DIR}/routes/" DESTINATION routes/)
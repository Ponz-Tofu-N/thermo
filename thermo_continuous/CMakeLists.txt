cmake_minimum_required(VERSION 3.16)

set(THREADS_PREFER_PTHREAD_FLAG ON)              
set (CMAKE_CXX_STANDARD 17)
project(thermo_c)

find_library (BCM2835_LIBRARY NAMES bcm2835)
# find_package(Threads REQUIRED)                   

set(SOURCES thermo_cont.cpp)
add_executable(thermo_c ${SOURCES})

target_link_libraries (thermo_c PUBLIC ${BCM2835_LIBRARY}  )
# target_link_libraries(led PUBLIC "-pthread") 

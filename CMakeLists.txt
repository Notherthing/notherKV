cmake_minimum_required(VERSION 3.10)

#project name
PROJECT(nother_kv)


#c11 standard
SET(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)     # Require C++17 support

SET(CMAKE_CXX_FLAGS "-static-libstdc++ -static-libgcc -O3 -ggdb ") 

find_package(Threads)

#src test lists
#SET(
#SKIPLIST_TEST
#test/skiplist_test.cpp
#)

#head file path
INCLUDE_DIRECTORIES(
include/
)

#lib file path
LINK_DIRECTORIES(
/libs
)

#set extern libraries
SET(LIBRARIES
)

# set output binary path
#SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/testdir)

#add executable file
#ADD_EXECUTABLE(skiplist_test ${SKIPLIST_TEST})

#add subdirectory
ADD_SUBDIRECTORY(test)

#add link library
#TARGET_LINK_LIBRARIES(skiplist_test ${LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} )

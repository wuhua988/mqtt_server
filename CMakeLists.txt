cmake_minimum_required(VERSION 2.8)

project(mqtt_server)

set(CMAKE_CXX_COMPILER            "g++")
set(CMAKE_CXX_FLAGS               "-Wall -W -std=c++11")
set(CMAKE_CXX_FLAGS_DEBUG         "-g")


message(" *PROJECT_SOURCE_DIR = " ${PROJECT_SOURCE_DIR})

if (CMAKE_BUILD_TYPE MATCHES "DEBUG")
        set( CMAKE_BUILD_TYPE DEBUG)
        message(" *ENABLE " ${CMAKE_BUILD_TYPE} " MODE")
endif()

set(LIB_DIR ${PROJECT_SOURCE_DIR}/lib/)

message(" *LIB_DIR =" ${LIB_DIR} )

set(SRC_DIR ${PROJECT_SOURCE_DIR}/src)
message(" *SRC_DIR =" ${SRC_DIR} )

set(BIN_DIR ${PROJECT_SOURCE_DIR}/bin)
message(" *BIN_DIR =" ${BIN_DIR} )

set(INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/include/common ${PROJECT_SOURCE_DIR}/include/msgpack)
message(" *INCLUDE_DIR = " ${INCLUDE_DIR})

include_directories(${INCLUDE_DIR})

link_directories(${PROJECT_SOURCE_DIR}/lib)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")

set(EXECUTABLE_OUTPUT_PATH ${BIN_DIR})

set(LIBRARY_OUTPUT_PATH ${LIB_DIR})

set(CMAKE_OPTION "${CMAKE_OPTION}")

ADD_SUBDIRECTORY(src)

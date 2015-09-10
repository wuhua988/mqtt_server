cmake_minimum_required(VERSION 2.8)

#SET(HTTP_SRC *.cpp)
aux_source_directory(. HTTP_SRC)

ADD_LIBRARY(http_common STATIC ${HTTP_SRC})
INSTALL(TARGETS http_common
            ARCHIVE DESTINATION ${PROJECT_SOURCE_DIR}/lib)


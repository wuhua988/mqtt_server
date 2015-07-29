cmake_minimum_required(VERSION 2.8)

aux_source_directory(. SOURCES) 

ADD_LIBRARY(common STATIC ${SOURCES})
INSTALL(TARGETS common
            ARCHIVE DESTINATION ${PROJECT_SOURCE_DIR}/lib)


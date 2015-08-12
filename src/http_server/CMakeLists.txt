aux_source_directory(. HTTP_SERVER_SRC)
ADD_LIBRARY(http_server STATIC ${HTTP_SERVER_SRC})
INSTALL(TARGETS http_server
                    ARCHIVE DESTINATION ${PROJECT_SOURCE_DIR}/lib)


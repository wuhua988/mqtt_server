cmake_minimum_required(VERSION 2.8)


#SET(REACTOR_SRC  event_handler.cpp poller_epoll.cpp sock_acceptor.cpp)
aux_source_directory(. REACTOR_SRC)

ADD_LIBRARY(reactor STATIC ${REACTOR_SRC})

INSTALL(TARGETS reactor
                ARCHIVE DESTINATION ${PROJECT_SOURCE_DIR}/lib)

cmake_minimum_required(VERSION 2.8)


SET(REACTOR_SRC  event_handler.cpp poller_epoll.cpp sock_acceptor.cpp)

ADD_LIBRARY(reactor STATIC ${REACTOR_SRC})

INSTALL(TARGETS reactor
                ARCHIVE DESTINATION ${PROJECT_SOURCE_DIR}/lib)

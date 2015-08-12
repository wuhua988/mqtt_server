add_definitions(-std=c++11 -g  -DHAVE_EPOLL ${CMAKE_OPTION})
set(CMAKE_EXE_LINKER_FLAGS "-lpthread")
#aux_source_directory(. SOURCES)
set(DB_SOURCES db_dump.cpp)

add_executable(db_dump ${DB_SOURCES})

TARGET_LINK_LIBRARIES(db_dump server)
TARGET_LINK_LIBRARIES(db_dump common) 
TARGET_LINK_LIBRARIES(db_dump reactor)                                                          
TARGET_LINK_LIBRARIES(db_dump mqttc++)                                                          
TARGET_LINK_LIBRARIES(db_dump log4cplus)

#TARGET_LINK_LIBRARIES(db_dump reactor)
# here for profile analysis
#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -ltcmalloc -lprofiler")

TARGET_LINK_LIBRARIES(db_dump ${CMAKE_EXE_LINKER_FLAGS})



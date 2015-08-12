add_definitions(-std=c++11 -g  -DHAVE_EPOLL ${CMAKE_OPTION})
set(CMAKE_EXE_LINKER_FLAGS "-lpthread")
aux_source_directory(. SOURCES)
add_executable(mqtt_broker ${SOURCES})

TARGET_LINK_LIBRARIES(mqtt_broker http_server)
TARGET_LINK_LIBRARIES(mqtt_broker server) 
TARGET_LINK_LIBRARIES(mqtt_broker common)
TARGET_LINK_LIBRARIES(mqtt_broker reactor)
TARGET_LINK_LIBRARIES(mqtt_broker mqttc++)
TARGET_LINK_LIBRARIES(mqtt_broker log4cplus)  

# here for profile analysis
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -ltcmalloc -lprofiler")

TARGET_LINK_LIBRARIES(mqtt_broker ${CMAKE_EXE_LINKER_FLAGS})



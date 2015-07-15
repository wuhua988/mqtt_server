
add_definitions(-std=c++11 -g  -DHAVE_EPOLL ${CMAKE_OPTION})
set(CMAKE_EXE_LINKER_FLAGS "-lpthread")
aux_source_directory(. SOURCES)

message("TCPServer SOURCES = " ${SOURCES})

add_executable(mqtt_server ${SOURCES})

TARGET_LINK_LIBRARIES(mqtt_server log4cplus)
TARGET_LINK_LIBRARIES(mqtt_server mqttc++)
TARGET_LINK_LIBRARIES(mqtt_server reactor)

TARGET_LINK_LIBRARIES(mqtt_server ${CMAKE_EXE_LINKER_FLAGS})



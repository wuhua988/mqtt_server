
add_definitions(-std=c++11 -g  -DHAVE_EPOLL ${CMAKE_OPTION})
set(CMAKE_EXE_LINKER_FLAGS "-lpthread")
aux_source_directory(. SOURCES)

message("mqtt client SOURCES = " ${SOURCES})

add_executable(mqtt_cli ${SOURCES})

TARGET_LINK_LIBRARIES(mqtt_cli log4cplus)
TARGET_LINK_LIBRARIES(mqtt_cli mqttc++)
#TARGET_LINK_LIBRARIES(mqtt_cli reactor)

TARGET_LINK_LIBRARIES(mqtt_cli ${CMAKE_EXE_LINKER_FLAGS})



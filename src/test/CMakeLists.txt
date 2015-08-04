
add_definitions(-std=c++11 -g  -DHAVE_EPOLL ${CMAKE_OPTION})
set(CMAKE_EXE_LINKER_FLAGS "-lpthread")
#aux_source_directory(. XML_SOURCES)
set(XML_SOURCES test_xml.cpp)

add_executable(test_xml ${XML_SOURCES})

TARGET_LINK_LIBRARIES(test_xml common)
TARGET_LINK_LIBRARIES(test_xml log4cplus)
#TARGET_LINK_LIBRARIES(mqtt_server mqttc++)
#TARGET_LINK_LIBRARIES(mqtt_server reactor)

# here for profile analysis
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -ltcmalloc -lprofiler")
TARGET_LINK_LIBRARIES(test_xml ${CMAKE_EXE_LINKER_FLAGS})



cmake_minimum_required(VERSION 2.8)

SET(LIBMQTTC++_SRC mqtt_msg.cpp mqtt_pkt.cpp)

ADD_LIBRARY(mqttc++ STATIC ${LIBMQTTC++_SRC})
INSTALL(TARGETS mqttc++
            ARCHIVE DESTINATION ${PROJECT_SOURCE_DIR}/lib)


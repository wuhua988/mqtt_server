cmake_minimum_required(VERSION 2.6)

SET(LIBMQTT_SRC mqtt_pkt_ansi.c helper.c)
ADD_LIBRARY(mqtt STATIC ${LIBMQTT_SRC})
INSTALL(TARGETS mqtt
            ARCHIVE DESTINATION lib)


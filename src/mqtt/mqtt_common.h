#ifndef _mqtt_pkt_common_h__
#define _mqtt_pkt_common_h__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef unsigned int bool;
typedef unsigned char byte;

#define DLOG_DEBUG my_printf
#define UNUSED(x) x


#define false 0
#define true 1

#define NEED_MORE_DATA  -2
#define FAILURE         -1
#define SUCCESS         0


typedef void* (*pf)(byte, char*, size_t);

enum msgTypes
{
    CONNECT = 1, CONNACK, PUBLISH, PUBACK, PUBREC, PUBREL,
    PUBCOMP, SUBSCRIBE, SUBACK, UNSUBSCRIBE, UNSUBACK,
    PINGREQ, PINGRESP, DISCONNECT
};


typedef union
{
    byte all;                         /**< the whole byte */
    struct
    {
        byte retain : 1;
        byte qos : 2;
        byte dup : 1;
        byte type : 4;
        
    } bits;
    
} TMqttHeader;

typedef struct
{
    TMqttHeader header;                 /**< MQTT header byte */
    int remain_length;
    union
    {
        char all;
        struct
        {
            byte : 1; /**< unused */
            byte clean_session : 1;
            byte will : 1;
            byte will_qos : 2;
            byte will_retail : 1;
            byte password : 1;
            byte username : 1;
        } bits;
    } flags;                            /**< connect flags byte */
    
    byte protocol_ver;
    char            *protocol_name;
    char            *client_id;
    char            *will_topic;
    char            *will_msg;
    char            *user_name;
    char            *user_passwd;
    
    unsigned short keep_alive_timer;         /**< keepalive timeout value in seconds */
} TMqttConnect;

typedef struct
{
    char *topic_name;
    int topic_qos;
} TMqttTopic;

/*
 typedef struct
 {
 TMqttTopic *next;
 TMqttTopic mqtt_topic;
 }TTopicList;
 */

#define  MAX_TOPIC_NUM 5
typedef struct
{
    TMqttHeader header;                         /**< MQTT header byte */
    int remain_length;
    
    int msg_id;
    TMqttTopic topics[5];                   /**< list of topic strings */
    int topics_count;                           /**< topic count */
} TMqttSubcribe;

typedef struct
{
    TMqttHeader header;                   /**< MQTT header byte */
    int remain_length;
    char            *topic_name;
    int msg_id_offset;
    int msg_id;
    
    int payload_len;
    char            *payload;
    
} TMqttPublish;

/**
 * Data for one of the ack packets.
 */
typedef struct
{
    TMqttHeader header; /**< MQTT header byte */
    int msg_id;      /**< MQTT message id */
} TAck;

typedef TAck TPuback;
typedef TAck TPubrec;
typedef TAck TPubrel;
typedef TAck TPubcomp;
typedef TAck TUnsuback;

typedef TMqttSubcribe TMqttUnsubcribe;

#endif

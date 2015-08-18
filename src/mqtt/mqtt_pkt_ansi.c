#include "mqtt_pkt_ansi.h"

#ifdef __cplusplus
extern "C" {
#endif
    
    /**
     caller should check len, len should >= 2
     return total msg len
     */
    
    int mqtt_dec_connect(const char *buf, int len, TMqttConnect **mqtt_connect_msg)
    {
        int offset = 0;
        int remain_length_value = -1;
        int remain_length_bytes = 0;
        
        int pkt_end_offset = 0;
        
        
        byte protocol_ver = 0;
        /* byte conn_flags = 0; */
        
        unsigned short protocol_name_len    = 0;
        unsigned short client_id_len        = 0;
        unsigned short will_topic_len       = 0;
        unsigned short will_msg_len         = 0;
        unsigned short user_name_len        = 0;
        unsigned short user_passwd_len      = 0;
        
        const char *p_proto_name    = NULL;
        const char *p_client_id     = NULL;
        const char *p_will_topic    = NULL;
        const char *p_will_msg      = NULL;
        const char *p_user_name     = NULL;
        const char *p_user_passwd   = NULL;
        
        int res = 0;
        
        const byte mqtt_version = 3;
        
        memset(*mqtt_connect_msg, 0, sizeof(TMqttConnect));
        
        /* 1. Get msg header */
        (*mqtt_connect_msg)->header.all = buf[offset]; /* get connect msg header and skip */
        offset++;
        
        if ((*mqtt_connect_msg)->header.bits.type != CONNECT)
        {
            DLOG_DEBUG("MQTT::Connect Msg, NOT CONNECT Type type %d", (*mqtt_connect_msg)->header.bits.type);
            return FAILURE;
        }
        
        
        /* 2. Get remain length */
        if ((res = mqtt_remain_length(buf + offset, len - offset, &remain_length_value, &remain_length_bytes)) < 0)
        {
            DLOG_DEBUG("MQTT::Connect Msg, get remain length failed, res %d! offset %d", res, offset);
            return res; /* failure or need more data */
        }
        
        DLOG_DEBUG("MQTT::Connect Msg, remain_length 0x%x, bytes %d, offset %d",
                   remain_length_value, remain_length_bytes, offset);
        
        offset += remain_length_bytes; /* remain_length_bytes */
        
        pkt_end_offset = remain_length_value + 1 + remain_length_bytes; /* 1: fix header */
        
        /* 3. decode protocol name*/
        
        if ( (res = get_string(buf + offset, len - offset, &p_proto_name, &protocol_name_len)) < 0)
        {
            DLOG_DEBUG("Get protocol msg failed. offset %d", offset);
            return FAILURE;
        }
        
        if (check_protocol_name(p_proto_name, protocol_name_len) < 0)
        {
            DLOG_DEBUG("Check protocol name failed.");
            return FAILURE;
        }
        
        offset += res; /* skip short and content */
        
        /* 4. protocol version */
        protocol_ver = buf[offset];
        
        if (protocol_ver != mqtt_version)
        {
            DLOG_DEBUG("Wrong protocol version should 0x%x, actuall value 0x%02x, offset %d",
                       mqtt_version,  protocol_ver, offset);
            return FAILURE;
        }
        offset++; /* skip protocol version */
        (*mqtt_connect_msg)->protocol_ver = protocol_ver;
        
        
        /* 5. Get conn flag */
        (*mqtt_connect_msg)->flags.all = buf[offset];
        offset++;
        
        /* 6. Get keep aliver timer value, short value */
        (*mqtt_connect_msg)->keep_alive_timer = get_short(buf + offset, len - offset);
        offset += 2;
        
        /* 7. Get client id */
        client_id_len = get_short(buf + offset, len - offset);
        offset += 2;
        
        /* In mqtt version 3.1, client_id_len most 23 bytes, but in 3.1.1 can maxinum to 65535 */
        p_client_id = buf + offset;
        offset += client_id_len;
        
        /* 8. Get Will msg and will topic */
        if ( (*mqtt_connect_msg)->flags.bits.will )
        {
            if ((res = get_string(buf + offset, len - offset, &p_will_msg, &will_msg_len)) < 0)
            {
                DLOG_DEBUG("Get will msg failed.");
                return FAILURE;
            }
            offset += res;
            
            if ((res = get_string(buf + offset, len - offset, &p_will_topic, &will_topic_len)) < 0)
            {
                DLOG_DEBUG("Get will msg failed");
                return FAILURE;
            }
            
            offset += res;
        }
        
        /* 9. Get User name */
        if ( (*mqtt_connect_msg)->flags.bits.username)
        {
            if ((res = get_string(buf + offset, len - offset, &p_user_name, &user_name_len)) < 0)
            {
                DLOG_DEBUG("Get Username failed");
                return FAILURE;
            }
            
            offset += res;
        }
        
        /* 10. Get Passwd */
        if ( (*mqtt_connect_msg)->flags.bits.password)
        {
            if ((res = get_string(buf + offset, len - offset, &p_user_passwd, &user_passwd_len)) < 0)
            {
                DLOG_DEBUG("Get passwd failed");
                return FAILURE;
            }
            
            offset += res;
        }
        
        if (offset > len || offset > pkt_end_offset)
        {
            DLOG_DEBUG("MQTT::Connect Msg, last offset(%d) > len (%d), something wrong", offset, len);
            return FAILURE;
        }
        
        (*mqtt_connect_msg)->remain_length = remain_length_value;
        
        (*mqtt_connect_msg)->protocol_name  = new_string(p_proto_name, protocol_name_len);
        (*mqtt_connect_msg)->client_id      = new_string(p_client_id, client_id_len);
        
        if ( (*mqtt_connect_msg)->flags.bits.will )
        {
            (*mqtt_connect_msg)->will_msg   = new_string(p_will_msg, will_msg_len);
            (*mqtt_connect_msg)->will_topic = new_string(p_will_topic, will_topic_len);
        }
        
        if ( (*mqtt_connect_msg)->flags.bits.username )
        {
            (*mqtt_connect_msg)->user_name = new_string(p_user_name, user_name_len);
        }
        
        if ( (*mqtt_connect_msg)->flags.bits.password )
        {
            (*mqtt_connect_msg)->user_passwd = new_string(p_user_passwd, user_passwd_len);
        }
        
        return offset; /* return total msg len */
    }
    
    int mqtt_enc_connect_ack(char *buf, int len, byte reason)
    {
        if (len < 4)
        {
            return FAILURE;
        }
        
        /**
         0	0x00	Connection Accepted
         1	0x01	Connection Refused: unacceptable protocol version
         2	0x02	Connection Refused: identifier rejected
         3	0x03	Connection Refused: server unavailable
         4	0x04	Connection Refused: bad user name or password
         5	0x05	Connection Refused: not authorized
         */
        
        buf[0] = (CONNACK<<4)&0xFF; /* header */
        buf[1] = 0x02;           /* len */
        
        buf[2] = 0x0;            /* reserved */
        buf[3] = reason;         /* reason */
        
        return 4;
    }
    
    int mqtt_dec_publish(const char *buf, int len, TMqttPublish **mqtt_publish)
    {
        int offset = 0;
        int remain_length_value = -1;
        int remain_length_bytes = 0;
        
        int pkt_end_offset = 0;
        
        const char *p_topic_name = NULL;
        unsigned short topic_len = 0;
        
        int payload_len = 0;
        
        int res = 0;
        
        /* 1. Get msg header */
        (*mqtt_publish)->header.all = buf[offset]; /* get connect msg header and skip */
        offset++;
        
        if ((*mqtt_publish)->header.bits.type != PUBLISH)
        {
            DLOG_DEBUG("MQTT::Connect Msg, NOT PUBLISH Type");
            return FAILURE;
        }
        
        /* 2. Get remain length */
        if ((res = mqtt_remain_length(buf + offset, len - offset, &remain_length_value, &remain_length_bytes)) < 0)
        {
            DLOG_DEBUG("MQTT::Publish Msg, get remain length failed, res %d! offset %d", res, offset);
            return res; /* failure or need more data */
        }
        
        DLOG_DEBUG("MQTT::Pubish Msg, remain_length 0x%x, bytes %d, offset %d",
                   remain_length_value, remain_length_bytes, offset);
        
        offset += remain_length_bytes; /* remain_length_bytes */
        
        pkt_end_offset = remain_length_value + 1 + remain_length_bytes; /* 1: fix header */
        
        /* 3. Get topic name*/
        if ((res = get_string(buf + offset, len - offset, &p_topic_name, &topic_len)) < 0)
        {
            DLOG_DEBUG("Get topic name failed");
            return FAILURE;
        }
        
        (*mqtt_publish)->topic_name = new_string(p_topic_name, topic_len);
        
        offset += res;
        
        /* 4. Get msg id */
        (*mqtt_publish)->msg_id_offset = offset;
        (*mqtt_publish)->msg_id = get_short(buf + offset, len - offset);
        offset += 2;
        
        (*mqtt_publish)->remain_length = remain_length_value;
        
        /* 5. Get payload */
        payload_len = pkt_end_offset - offset;
        
        (*mqtt_publish)->payload = new_string(buf + offset,payload_len);
        (*mqtt_publish)->payload_len = payload_len;
        
        return pkt_end_offset;
    }
    
    /*
     int mqtt_enc_publish(char *buf, int len, TMqttPublish *mqtt_publish)
     {
     int offset = 0;
     int res = 0;
     
     // header +1
     // len  = strlen(topic_name) + 2 + msg_id(2) + payload(payload_len)
     // topic_name
     res = write_string(buf+offset, len - offset, mqtt_publish->topic_name);
     offset += res;
     // msg_id
     write_short(buf+offset, len - offset);
     
     // payload
     memcpy(buf+offset, mqtt_publish->payload, mqtt_publish->payload_len);
     
     return offset + payload_len;
     }
     */
    
    int mqtt_enc_publish_ack(char *buf, int len, unsigned msg_id)
    {
        int offset = 0;
        
        if (len < 4)
        {
            return FAILURE;
        }
        
        buf[offset] = (PUBACK<<4)&0xFF; /* header */
        offset++;
        
        buf[offset] = 2;                /* 2: msg_id */
        offset++;
        
        write_short(buf+offset, len - offset, msg_id);
        
        return 4;
        
    }
    
    int mqtt_dec_subscribe(const char *buf, int len, TMqttSubcribe **mqtt_subcribe, bool subcribe)
    {
        int offset = 0;
        int remain_length_value = -1;
        int remain_length_bytes = 0;
        
        int pkt_end_offset = 0;
        
        const char *p_topic_name = NULL;
        unsigned short topic_len = 0;
        char topic_qos = 0;
        int topic_index = 0;
        
        
        int res = 0;
        
        /* 1. Get msg header */
        (*mqtt_subcribe)->header.all = buf[offset]; /* get connect msg header and skip */
        offset++;
        
        if ((*mqtt_subcribe)->header.bits.type != SUBSCRIBE)
        {
            DLOG_DEBUG("MQTT::Connect Msg, NOT SUBSCRIBE Type");
            return FAILURE;
        }
        
        /* 2. Get remain length */
        if ((res = mqtt_remain_length(buf + offset, len - offset, &remain_length_value, &remain_length_bytes)) < 0)
        {
            DLOG_DEBUG("MQTT::Subscribe Msg, get remain length failed, res %d! offset %d", res, offset);
            return res; /* failure or need more data */
        }
        
        DLOG_DEBUG("MQTT::Subscribe Msg, remain_length 0x%x, bytes %d, offset %d",
                   remain_length_value, remain_length_bytes, offset);
        
        offset += remain_length_bytes; /* remain_length_bytes */
        
        pkt_end_offset = remain_length_value + 1 + remain_length_bytes; /* 1: fix header */
        
        /* 3. Get msg id */
        (*mqtt_subcribe)->msg_id = get_short(buf + offset, len - offset);
        offset += 2;
        
        /*4. Get topics */
        (*mqtt_subcribe)->topics_count = 0;
        while ((offset < len) && (offset < pkt_end_offset))
        {
            if ((res = get_string(buf + offset, len - offset, &p_topic_name, &topic_len)) < 0)
            {
                DLOG_DEBUG("Get topic name failed");
                return FAILURE;
            }
            
            offset += res;
            
            if (subcribe) /* Unsubcriber no qos*/
            {
                topic_qos = buf[offset]&0x3; /* low two bit */
                offset += 1;
            }
            
            if ((*mqtt_subcribe)->topics_count > MAX_TOPIC_NUM - 1)
            {
                DLOG_DEBUG("Too many topic");
                break;
            }
            
            topic_index = (*mqtt_subcribe)->topics_count;
            (*mqtt_subcribe)->topics[topic_index].topic_name = new_string(p_topic_name, topic_len);
            (*mqtt_subcribe)->topics[topic_index].topic_qos  = topic_qos;
            
            (*mqtt_subcribe)->topics_count++;
        }
        
        (*mqtt_subcribe)->remain_length = remain_length_value;
        
        return offset;
    }
    
    
    int mqtt_enc_unsubscribe_ack(char *buf, int len, unsigned short msg_id)
    {
        int offset = 0;
        if (len < 4)
        {
            return FAILURE;
        }
        
        buf[offset] = (UNSUBACK<<4)&0xFF; /* header */
        offset++;
        
        buf[offset] = 2;                /* 2: msg_id */
        offset++;
        
        write_short(buf+offset, len - offset, msg_id);
        
        return 4;
    }
    
    int mqtt_enc_subscribe_ack(char *buf, int len, unsigned short msg_id, int *topics_qos, int size)
    {
        int offset = 0;
        int i = 0;
        int total_lenght = 0;
        
        if (len < 4)
        {
            return FAILURE;
        }
        
        buf[offset] = (SUBACK<<4)&0xFF; /* header */
        offset++;
        
        buf[offset] = 2 + size;           /* 2: msg_id */
        offset++;
        
        total_lenght = 4 + size; /* fix_header(1) remain_lenght(1) msg_id(2)  qos(size)*/
        
        if ( len < total_lenght) /* total msg lenght */
        {
            DLOG_DEBUG("Not enough buf for mqtt_enc_subscribe_ack");
            return FAILURE;
        }
        
        write_short(buf+offset, len - offset, msg_id);
        offset += 2;
        
        
        for (i = 0; i < size; i++)
        {
            buf[offset] = topics_qos[i];
            offset++;
        }
        
        return total_lenght;
    }
    
#ifdef __cplusplus
}
#endif


#include "helper.h"
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

int my_printf(char *fmt, ...)
{
    char buffer[1024];
    va_list argptr;
    int length = 0;

    va_start(argptr, fmt);
    length = vsnprintf(buffer,1024 ,fmt, argptr);
    va_end(argptr);

    /*
       buffer[length] = '\n';
       buffer[length + 1] = '\0';
       */

    printf("%s\n", buffer);

    return (length + 1);
}

void print_hex_dump(const char *buf, int len)
{
    /* char buf[1024]; */
    int i = 0;

    printf("0000 ");
    for (; i < len; i++)
    {
        if (i && ((i%16) == 0))
        {
            printf("\n%04x ", i);
        }

        printf("%02X ", (byte) buf[i]);
    }

    printf("\n");
}

/*
   ping pong disconnect, remain_lenght = 0
   */


void print_connect(TMqttConnect *mqtt_connect_msg)
{
    DLOG_DEBUG("MQTTConnect Msg Detail");
    DLOG_DEBUG("FixHeader %x, Type %d, DUP %d, QOS %d, RETAIN %d, RemainLength %d",
            mqtt_connect_msg->header.bits,
            mqtt_connect_msg->header.bits.type,
            mqtt_connect_msg->header.bits.dup,
            mqtt_connect_msg->header.bits.qos,
            mqtt_connect_msg->header.bits.retain,
            mqtt_connect_msg->remain_length);

    DLOG_DEBUG("Protocol Name: \t%s, Version %d", mqtt_connect_msg->protocol_name, mqtt_connect_msg->protocol_ver);
    DLOG_DEBUG("ClientId: \t%s", mqtt_connect_msg->client_id);
    DLOG_DEBUG("WillTopic: \t%s",(mqtt_connect_msg->will_topic == NULL)?"NULL":mqtt_connect_msg->will_topic);
    DLOG_DEBUG("WillMsg: \t%s", (mqtt_connect_msg->will_msg == NULL)?"NULL":mqtt_connect_msg->will_msg);
    DLOG_DEBUG("UserName: \t%s", (mqtt_connect_msg->user_name == NULL)?"NULL":mqtt_connect_msg->user_name);
    DLOG_DEBUG("PassWord: \t%s\n", (mqtt_connect_msg->user_passwd == NULL)?"NULL":mqtt_connect_msg->user_passwd);
}

void print_subcribe(TMqttSubcribe *mqtt_subcribe_msg)
{
    int i = 0;
    DLOG_DEBUG("MQTTSubcribe Msg Detail");
    DLOG_DEBUG("FixHeader %x, Type %d, DUP %d, QOS %d, RETAIN %d, RemainLength %d",
            mqtt_subcribe_msg->header.bits,
            mqtt_subcribe_msg->header.bits.type,
            mqtt_subcribe_msg->header.bits.dup,
            mqtt_subcribe_msg->header.bits.qos,
            mqtt_subcribe_msg->header.bits.retain,
            mqtt_subcribe_msg->remain_length);

    DLOG_DEBUG("Msg id %d\tTopics", mqtt_subcribe_msg->msg_id);

    for (i = 0; (i < mqtt_subcribe_msg->topics_count) && (i < MAX_TOPIC_NUM); i++)
    {
        DLOG_DEBUG("[%d] %s, qos %d\n", i+1, mqtt_subcribe_msg->topics[i].topic_name, mqtt_subcribe_msg->topics[i].topic_qos);
    }

    DLOG_DEBUG("\n");
}

void print_publish(TMqttPublish *mqtt_publish_msg)
{
    DLOG_DEBUG("MQTTPublish Msg Detail");
    DLOG_DEBUG("FixHeader %x, Type %d, DUP %d, QOS %d, RETAIN %d, RemainLength %d",
            mqtt_publish_msg->header.bits,
            mqtt_publish_msg->header.bits.type,
            mqtt_publish_msg->header.bits.dup,
            mqtt_publish_msg->header.bits.qos,
            mqtt_publish_msg->header.bits.retain,
            mqtt_publish_msg->remain_length);

    DLOG_DEBUG("Topic name : %s", mqtt_publish_msg->topic_name);
    DLOG_DEBUG("Msg id: %d, offset %d", mqtt_publish_msg->msg_id, mqtt_publish_msg->msg_id_offset);
    DLOG_DEBUG("Payload len %d, [%s]\n", mqtt_publish_msg->payload_len, mqtt_publish_msg->payload);
}


bool has_remain_len(char value)
{
    return ((value & 0x80) == 0x80);
}

char *new_string(const char *str, int len) /* len not include '0' */
{
    char *p = (char *)malloc(len + 1);

    if (p != NULL)
    {
        memcpy(p, str, len);
        p[len] = '\0';
    }

    return p;
}

/** return value
  = 0 succeed
  -1 failed,
  -2 need more bytes

  return bytes of lenght
  */

int mqtt_remain_length(const char* buf, int len, int *rem_length_value, int *rem_length_bytes)
{
    int remin_length_value = 0;
    int offset = 0;
    bool last_byte_found = false;

    const int MAX_REMAIN_LENGTH_BYTES = 4;
    while((offset < MAX_REMAIN_LENGTH_BYTES) && (offset < len)) /* most remain lenght 4 bytes */
    {
        char byte_value = buf[offset];
        remin_length_value = (remin_length_value << 7)|(byte_value&0x7F);

        if (!has_remain_len(byte_value)) /* the last bytes */
        {
            last_byte_found = true;
            break;
        }

        offset++;
    }

    if (!last_byte_found) /* no last byte */
    {
        return ((len >= MAX_REMAIN_LENGTH_BYTES)? FAILURE: NEED_MORE_DATA);
    }

    /* we got rem length value */
    *rem_length_value = remin_length_value;
    *rem_length_bytes = offset + 1;

    if ((remin_length_value + offset) > len)  /* not full pkt we got */
    {
        return NEED_MORE_DATA;
    }

    return SUCCESS;
}

unsigned short get_short(const char *buf, int len)
{
    unsigned short short_msb = 0;

    if (len < 2)
    {
        return 0;
    }

    short_msb = (byte)buf[0];
    short_msb =  (short_msb << 8) | ((byte)buf[1]&0xFF);

    DLOG_DEBUG("Get Short 0x%02x 0x%02x => 0x%02x", (byte)buf[0], (byte)buf[1], short_msb);
    return short_msb;
}

int write_short(char *buf, int len, unsigned short msg_id)
{
    if (len < 2)
    {
        return FAILURE;
    }

    buf[0] = (msg_id >> 8)&0xFF;
    buf[1] = msg_id&0xFF;

    return 2;
}

/* return offset total */
int get_string(const char *buf, int len, const char **pstart, unsigned short *str_len) /* lenght is short */
{
    int total_len = 0;

    *str_len = get_short(buf, len);
    if (*str_len > (unsigned short)len)
    {
        DLOG_DEBUG("Msg part len (%d) > total len (%d)", *str_len, len);
        return -1;
    }

    total_len = *str_len + 2; /* +2 skip short len */

    *pstart = buf + 2;

    return total_len; /* +2 skip short len */
}

int write_string(char *buf, int len, const char *str, unsigned short str_len)
{
    int i = 0;
    if (len < str_len + 2) /* 2 for str_len */
    {
        DLOG_DEBUG("Write string, not enough room for str");
        return FAILURE;
    }

    write_short(buf, len, str_len);
    for (i = 0; i < str_len; i++)
    {
        buf[2+i] = str[i];
    }

    return str_len + 2;
}

int check_protocol_name(const char *pstart, int protocol_name_len)
{
    /* "Mqisdp"  v3.1,  "MQTT"  v3.1.1 */
    const char *mqtt_3_1_pro_name = "MQIsdp";
    const int  mqtt_3_1_pro_len = 6;

    const char *mqtt_3_1_1_pro_name = "MQTT";
    const int  mqtt_3_1_1_pro_len = 4;

    if (protocol_name_len == mqtt_3_1_pro_len)
    {
        if (memcmp(pstart, mqtt_3_1_pro_name,mqtt_3_1_pro_len) != 0)
        {
            DLOG_DEBUG("protocol name should %s, in version 3.1", mqtt_3_1_pro_name);
            return FAILURE;
        }

    }
    else if (protocol_name_len == mqtt_3_1_1_pro_len)
    {
        if (memcmp(pstart, mqtt_3_1_1_pro_name,mqtt_3_1_1_pro_len) != 0)
        {
            DLOG_DEBUG("protocol name should %s, in version 3.1.1", mqtt_3_1_1_pro_name);

            return FAILURE;
        }
    }
    else
    {
        DLOG_DEBUG("Unknown protocol name len %d", protocol_name_len);
        return FAILURE;
    }

    return SUCCESS;
}

void free_mqtt_connect(TMqttConnect *mqtt_connect_msg)
{
    if (mqtt_connect_msg->protocol_name != NULL)
    {
        free(mqtt_connect_msg->protocol_name);
        mqtt_connect_msg->protocol_name = NULL;
    }

    if (mqtt_connect_msg->client_id != NULL)
    {
        free(mqtt_connect_msg->client_id);
        mqtt_connect_msg->client_id = NULL;
    }

    if (mqtt_connect_msg->will_topic != NULL)
    {
        free(mqtt_connect_msg->will_topic);
        mqtt_connect_msg->will_topic = NULL;
    }

    if (mqtt_connect_msg->will_msg != NULL)
    {
        free(mqtt_connect_msg->will_msg);
        mqtt_connect_msg->will_msg = NULL;
    }

    if (mqtt_connect_msg->user_name != NULL)
    {
        free(mqtt_connect_msg->user_name);
        mqtt_connect_msg->user_name = NULL;
    }

    if (mqtt_connect_msg->user_passwd != NULL)
    {
        free(mqtt_connect_msg->user_passwd);
        mqtt_connect_msg->user_passwd = NULL;
    }

}

void free_mqtt_subscribe(TMqttSubcribe *mqtt_subcribe_msg)
{
    int i = 0;
    for (i = 0; i < mqtt_subcribe_msg->topics_count; i++)
    {
        if (mqtt_subcribe_msg->topics[i].topic_name != NULL)
        {
            free(mqtt_subcribe_msg->topics[i].topic_name);
            mqtt_subcribe_msg->topics[i].topic_name = NULL;
        }
    }
}

void free_publish(TMqttPublish *mqtt_publish_msg)
{
    if (mqtt_publish_msg->topic_name != NULL)
    {
        free(mqtt_publish_msg->topic_name);
        mqtt_publish_msg->topic_name = NULL;
    }

    if (mqtt_publish_msg->payload != NULL)
    {
        free(mqtt_publish_msg->payload);
        mqtt_publish_msg->payload = NULL;
    }
}

#ifdef __cplusplus
}
#endif


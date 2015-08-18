#ifndef _mqtt_helper_h__
#define _mqtt_helper_h__

#include "mqtt_common.h"

#ifdef __cplusplus
extern "C" {
#endif
    
    int my_printf(char *fmt, ...);
    void print_hex_dump(const char *buf, int len);
    
    bool has_remain_len(char value);
    char *new_string(const char *str, int len) /* len not include '0' */;
    
    unsigned short get_short(const char *buf, int len);
    int write_short(char *buf, int len, unsigned short msg_id);
    int get_string(const char *buf, int len, const char **pstart, unsigned short *str_len) /* lenght is short */;
    int write_string(char *buf, int len, const char *str, unsigned short str_len);
    
    int mqtt_remain_length(const char* buf, int len, int *rem_length_value, int *rem_length_bytes);
    int check_protocol_name(const char *pstart, int protocol_name_len);
    
    
    void print_connect(TMqttConnect *mqtt_connect_msg);
    void print_subcribe(TMqttSubcribe *mqtt_subcribe_msg);
    void print_publish(TMqttPublish *mqtt_publish_msg);
    
    
    void free_mqtt_connect(TMqttConnect *mqtt_connect_msg);
    void free_mqtt_subscribe(TMqttSubcribe *mqtt_subcribe_msg);
    void free_publish(TMqttPublish *mqtt_publish_msg);
    
#ifdef __cplusplus
}
#endif

#endif /* header */

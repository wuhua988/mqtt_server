#include "mqtt_common.h"
#include "helper.h"

#ifndef  _mqtt_pkt_ansi_h__
#define  _mqtt_pkt_ansi_h__

#ifdef __cplusplus
extern "C"
{
#endif
    
    int mqtt_dec_connect(const char *buf, int len, TMqttConnect **mqtt_connect_msg);
    int mqtt_enc_connect_ack(char *buf, int len, byte reason);
    
    int mqtt_dec_publish(const char *buf, int len, TMqttPublish **mqtt_publish);
    int mqtt_enc_publish_ack(char *buf, int len, unsigned msg_id);
    
    int mqtt_dec_subscribe(const char *buf, int len, TMqttSubcribe **mqtt_subcribe, bool subcribe);
    int mqtt_enc_unsubscribe_ack(char *buf, int len, unsigned short msg_id);
    
    int mqtt_enc_subscribe_ack(char *buf, int len, unsigned short msg_id, int *topics_qos, int size);
    
#ifdef __cplusplus
}
#endif

#endif

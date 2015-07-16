//
//  mqtt_msg.h
//  mqtt_c++
//
//  Created by davad.di on 7/13/15.
//  Copyright (c) 2015 davad.di. All rights reserved.
//

#ifndef __mqtt_c____mqtt_msg__
#define __mqtt_c____mqtt_msg__

#include <memory>
#include "reactor/define.hpp"
#include "mqtt_pkt.hpp"

enum class MqttType {
    RESERVED1   = 0,
    CONNECT     = 1,
    CONNACK     = 2,
    PUBLISH     = 3,
    PUBACK      = 4,
    PUBREC      = 5,
    PUBREL      = 6,
    PUBCOMP     = 7,
    SUBSCRIBE   = 8,
    SUBACK      = 9,
    UNSUBSCRIBE = 10,
    UNSUBACK    = 11,
    PINGREQ     = 12,
    PINGRESP    = 13,
    DISCONNECT  = 14,
    RESERVED2   = 15
};


typedef union
{
    uint8_t          all;
    struct
    {
        uint8_t      retain:1;
        uint8_t      qos:2;
        uint8_t      dup:1;
        uint8_t      msg_type:4;
    }bits;
}FixHeaderFlag;

typedef union
{
    uint8_t all;
    struct
    {
        uint8_t:1;                            /**< unused */
        uint8_t    clean_session:1;
        uint8_t    will:1;
        uint8_t    will_qos:2;
        uint8_t    will_retail:1;
        uint8_t    password:1;
        uint8_t    username:1;
    } bits;
}ConnectFlag;

class CMqttFixedHeader // encode and decode
{
public:
    // enum {SIZE = 2};
    CMqttFixedHeader()
    {
    }

    void msg_type(MqttType msg_type)
    {
	m_msg_type = msg_type;
    }

    MqttType msg_type()
    {
	return m_msg_type;
    }

    void dup_flag(bool dup_flag)
    {
	m_dup_flag = dup_flag;
    }
    
    bool dup_flag()
    {
	return m_dup_flag;
    }

    void retain_flag(bool retain_flag)
    {
	m_retain_flag = retain_flag;
    }

    bool retain_flag()
    {
	return m_retain_flag;
    }

    void qos(uint8_t qos)
    {
	m_qos = qos;
    }

    uint8_t qos()
    {
	return m_qos;
    }

    CMqttFixedHeader(MqttType msg_type)
	: m_msg_type(msg_type),m_dup_flag(0), m_qos(0), m_retain_flag(0)
    {
    }

    CMqttFixedHeader(MqttType msg_type, bool dup_flag, uint8_t qos, bool retain_flag)
                    : m_msg_type(msg_type),m_dup_flag(dup_flag), m_qos(qos), m_retain_flag(retain_flag)
    {
    }
    
    
    int decode(uint8_t value);
    int encode(uint8_t &fix_header);
    void print();
    
    // change to protected later
protected:
    MqttType    m_msg_type;
    bool        m_dup_flag;
    
    uint8_t     m_qos;
    bool        m_retain_flag;

};

//--------------------* Connect and Ack *---------------------------
class CMqttMsg
{
public:
    CMqttMsg(uint8_t *buf, int len):m_mqtt_pkt(buf, len) // for decode
    {
    }
    
    CMqttMsg(uint8_t *buf, int len, CMqttFixedHeader fixed_header)  // for encode
                    :m_fixed_header(fixed_header), m_mqtt_pkt(buf, len)
    {
    }
    
    // -1 failed. 0 success
    int decode();

    int encode();


protected:
    CMqttFixedHeader m_fixed_header;
    uint32_t         m_remain_length_value;
    uint8_t          m_remain_length_bytes;
    
    CMqttPkt         m_mqtt_pkt;
};

class CMqttConnect : public CMqttMsg    // decode
{
public:

    CMqttConnect(uint8_t *buf, int len) : CMqttMsg(buf, len)
    {
    }
    
    // -1 failed. 0 success
    int decode();
    
    void print();

    std::string & proto_name()
    {
	return m_str_proto_name;
    }

    uint8_t proto_version()
    {
	return m_proto_version;
    }

    bool has_user_name()
    {
	return m_has_user_name;
    }

    bool has_passwd()
    {
	return m_has_password;
    }


    bool has_will()
    {
	return m_has_will;
    }

    uint8_t will_qos()
    {
	return m_will_qos;
    }

    bool clean_session()
    {
	return m_clean_session;
    }

    uint16_t keep_alive()
    {
	return m_keep_alive;
    }

    std::string & client_id()
    {
	return m_str_client_id;
    }

    std::string & will_topic()
    {
	return m_str_will_topic;
    }

    std::string & will_message()
    {
	return m_str_will_message;
    }

    std::string & user_name()
    {
	return m_str_user_name;
    }

    std::string & password()
    {
	return m_str_password;
    }
    
protected:
    std::string      m_str_proto_name;
    uint8_t          m_proto_version;
    bool             m_has_user_name;       //1
    bool             m_has_password;        //1
    bool             m_has_will_retain;     //1
    uint8_t          m_will_qos;            //2
    bool             m_has_will;            //1
    bool             m_clean_session;       //1
    bool             m_reserved;            //1
    uint16_t         m_keep_alive;
    
    std::string      m_str_client_id;
    std::string      m_str_will_topic;
    std::string      m_str_will_message;
    std::string      m_str_user_name;
    std::string      m_str_password;
};

class CMqttConnAck : public CMqttMsg  // encode
{
public:
    enum class Code
    {
        ACCEPTED = 0,
        BAD_VERSION = 1,
        BAD_ID = 2,
        SERVER_UNAVAILABLE = 3,
        BAD_USER_OR_PWD = 4,
        NO_AUTH = 5,
    };
    
    CMqttConnAck(uint8_t *buf, int len, CMqttFixedHeader fixed_header, Code code)
                                            : CMqttMsg(buf, len, fixed_header),m_code(code)
    {
    }

    // return encode buf lenght
    int encode();
    void print();

public:
    uint8_t          m_reserved;
    Code             m_code;
};

//--------------------* Subscribe and Ack *---------------------------
class CTopic
{
public:
    CTopic():m_str_topic_name(""), m_qos(0)
    {
    }
    
    CTopic(std::string topic_name, uint8_t qos)
            :m_str_topic_name(std::move(topic_name)), m_qos(qos)
    {
    }
    
    bool operator ==(const CTopic& t) const
    {
        return (m_str_topic_name == t.m_str_topic_name && m_qos == t.m_qos);
    }

    std::string & topic_name()
    {
	return m_str_topic_name;
    }

    uint8_t qos()
    {
	return m_qos;
    }

protected:
    std::string     m_str_topic_name;
    uint8_t         m_qos;
};

class CMqttSubscribe : public CMqttMsg    // decode
{
public:
    
    CMqttSubscribe(uint8_t *buf, int len) : CMqttMsg(buf, len)
    {
    }
    
    int decode();
    void print();

    uint16_t msg_id()
    {
	return m_msg_id;
    }

    std::vector<CTopic> & sub_topics()
    {
	return m_sub_topics;
    }

    std::vector<std::string> topics_name()
    {
	std::vector<std::string> topics_name;

	for (auto it = m_sub_topics.begin(); it != m_sub_topics.end(); it++)
	{
	    topics_name.push_back(it->topic_name());
	}

	return topics_name;
    }

    std::vector<uint8_t> topics_qos()
    {
	std::vector<uint8_t> topics_qos;
	for (auto it = m_sub_topics.begin(); it != m_sub_topics.end(); it++)  
	{
	    topics_qos.push_back(it->qos());
	}

	return topics_qos;
    }

protected:
    uint16_t             m_msg_id;
    std::vector<CTopic>  m_sub_topics;
};

class CMqttSubAck : public CMqttMsg // encode
{
public:
    
    CMqttSubAck(uint8_t *buf, int len, CMqttFixedHeader fixed_header, uint16_t msg_id, std::vector<uint8_t> sub_qos)
        : CMqttMsg(buf, len, fixed_header), m_msg_id(msg_id), m_sub_qos(sub_qos)
    {
    }
    
    int encode();
    void print();

protected:
    uint16_t                m_msg_id;
    std::vector<uint8_t>    m_sub_qos;
};

//--------------------* UnSubscribe and Ack *---------------------------

class CMqttUnsubscribe : public CMqttMsg // decode
{
public:
    CMqttUnsubscribe(uint8_t *buf, int len):CMqttMsg(buf, len)
    {
    }
    
    int decode();
    void print();
 
    uint16_t msg_id()
    {
	return m_msg_id;
    }

    std::vector<std::string> & topics_name()
    {
	return m_str_topics;
    }

protected:
    uint16_t                    m_msg_id;
    std::vector<std::string>    m_str_topics;
};

class CMqttUnsubAck : public CMqttMsg // encode
{
public:
    
    CMqttUnsubAck(uint8_t *buf, int len, CMqttFixedHeader fixed_header, uint16_t msg_id)
                                            : CMqttMsg(buf, len, fixed_header), m_msg_id(msg_id)
    {
    }
    
    int encode();
    void print();

protected:
    uint16_t        m_msg_id;
};

//--------------------* Publish and Ack *---------------------------

class CMqttPublish : public CMqttMsg     // decode and encode
{
public:
    
    CMqttPublish(uint8_t *buf, int len):CMqttMsg(buf, len) // decode
    {
    }
    
    CMqttPublish(uint8_t *buf, int len, CMqttFixedHeader fixed_header):CMqttMsg(buf, len, fixed_header) // encode
    {
    }
    
    int decode(); 
    void print();

    std::string & topic_name()
    {
	return m_str_topic_name;
    }

    std::vector<uint8_t> &  payload()
    {
	return m_payload;
    }

    uint16_t msg_id()
    {
	return m_msg_id;
    }

protected:
    std::string             m_str_topic_name;
    std::vector<uint8_t>    m_payload;
    uint16_t                m_msg_id;
};


typedef CMqttUnsubAck CMqttPublishAck; 


/*
class CMqttDisconnect : public CMqttMsg   // no need decode, just base on msg_type
{
public:
    CMqttDisconnect(uint8_t *buf, int len):CMqttMsg(buf, len)
    {
    }
    
    int decode()
    {
        return 0; // no need to decode
    }
};


class CMqttPingReq       // no need decode, just base on msg_type
{
public:
    CMqttPingReq(uint8_t *buf, int len):CMqttMsg(buf, len)
    {
    }
};
 */

class CMqttPingResp : public CMqttMsg
{
public:
    CMqttPingResp(uint8_t *buf, int len, CMqttFixedHeader fixed_header):CMqttMsg(buf, len, fixed_header)
    {
    }
    
    int encode();
};

#endif /* defined(__mqtt_c____mqtt_msg__) */

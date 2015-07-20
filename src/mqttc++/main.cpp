//
//  main.cpp
//  mqtt_c++
//
//  Created by davad.di on 7/14/15.
//  Copyright (c) 2015 davad.di. All rights reserved.
//

#include "mqttc++/mqtt_msg.hpp"

void print_hex_dump(const uint8_t *buf, int len)
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

	printf("%02X ", buf[i]);
    }

    printf("\n");
}

int main()
{
    CLoggerMgr logger("log4cplus.properties");

    uint8_t connect_buf[] = {
	0x10, 0x43, 0x00, 0x06, 0x4d, 0x51, 0x49, 0x73,
	0x64, 0x70, 0x03, 0xee, 0x00, 0x3c, 0x00, 0x0c,
	0x64, 0x61, 0x76, 0x61, 0x64, 0x2e, 0x64, 0x69,
	0x5f, 0x63, 0x6c, 0x69, 0x00, 0x05, 0x68, 0x65,
	0x6c, 0x6c, 0x6f, 0x00, 0x0d, 0x68, 0x65, 0x6c,
	0x6c, 0x6f, 0x5f, 0x70, 0x61, 0x79, 0x6c, 0x6f,
	0x61, 0x64, 0x00, 0x07, 0x64, 0x61, 0x76, 0x61,
	0x2e, 0x64, 0x69, 0x00, 0x08, 0x70, 0x61, 0x73,
	0x73, 0x77, 0x6f, 0x72, 0x64 };


    uint8_t buf[128];

    CMqttFixedHeader fixed_header(MqttType::CONNECT);
    CMqttConnect conn(buf, 128, fixed_header);
    conn.client_id("hello");

    int len = conn.encode();
    print_hex_dump(buf, len);
/*
    CMqttConnect connect(connect_buf, sizeof(connect_buf));

    if (connect.decode() < 0)
    {
	LOG_DEBUG("Connect msg decode failed");
	return -1;
    }

    connect.print();

    uint8_t con_ack_buf[32];

    CMqttFixedHeader fixed_header(MqttType::CONNACK);
    CMqttConnAck con_ack(con_ack_buf, 32, fixed_header, CMqttConnAck::Code::ACCEPTED);

    int len = 0;
    if ((len = con_ack.encode()) < 0)
    {
	LOG_DEBUG("Con ack encode failed");
	return -1;
    }

    print_hex_dump(con_ack_buf, len);


    uint8_t sub_buf[] = {
	0x82, 0x18, 0x00, 0x01, 0x00, 0x13, 0x73, 0x65, 
	0x6e, 0x73, 0x6f, 0x72, 0x73, 0x2f, 0x74, 0x65, 
	0x6d, 0x70, 0x65, 0x72, 0x61, 0x74, 0x75, 0x72, 
	0x65, 0x01 };
    
    CMqttSubscribe sub(sub_buf, sizeof(sub_buf));

    if (sub.decode() < 0)
    {
	LOG_DEBUG("sub decode failed");
	return -1;
    }

    sub.print();

    fixed_header.m_msg_type = MqttType::SUBACK;
    std::vector<uint8_t>    sub_qos;
    for (auto it = sub.m_sub_topics.begin(); it != sub.m_sub_topics.end(); it++)
    {
	sub_qos.push_back(it->m_qos);
    }

    CMqttSubAck sub_ack(con_ack_buf, 32, fixed_header, sub.m_msg_id, sub_qos);

    if ((len = sub_ack.encode()) < 0)
    {
	LOG_DEBUG("sub ack encode failed");
	return -1;
    }

    print_hex_dump(con_ack_buf, len);  

    uint8_t pub_buf[] = {
	0x32, 0x19, 0x00, 0x13, 0x73, 0x65, 0x6e, 0x73, 
	0x6f, 0x72, 0x73, 0x2f, 0x74, 0x65, 0x6d, 0x70, 
	0x65, 0x72, 0x61, 0x74, 0x75, 0x72, 0x65, 0x00, 
	0x01, 0x33, 0x32 };

    CMqttPublish pub(pub_buf, sizeof(pub_buf));

    if (pub.decode() < 0)
    {
	LOG_DEBUG("Publish decode failed");
	return -1;
    }
    pub.print();

    fixed_header.m_msg_type = MqttType::PUBACK;
    CMqttPublishAck pub_ack(con_ack_buf, 32, fixed_header, pub.m_msg_id);
    if ((len = pub_ack.encode()) < 0) 
    {
	LOG_DEBUG("pub ack encode failed"); 
	return -1; 
    }

    print_hex_dump(con_ack_buf, len);
*/
    return 0;
}

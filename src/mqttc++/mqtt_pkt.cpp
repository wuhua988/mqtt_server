//
//  mqtt_pkt.cpp
//  mqtt_c++
//
//  Created by davad.di on 7/13/15.
//  Copyright (c) 2015 davad.di. All rights reserved.
//

#include "mqttc++/mqtt_pkt.hpp"
#include <string>


// -1 failed -2 need more data, > 0 remain_length_value
int remain_length(uint8_t *pkt_buf, uint32_t len, uint32_t &remain_length_value, uint8_t &remain_length_bytes)
{
    const int POS_START_OF_REMAIN_LENGTH = 1; // start by 1
    uint8_t *buf = pkt_buf + POS_START_OF_REMAIN_LENGTH;
    
    int offset = 0;
    bool last_byte_found = false;

    remain_length_value = 0;
    remain_length_bytes = 0;

    const int MAX_REMAIN_LENGTH_BYTES = 4;
    while(offset < MAX_REMAIN_LENGTH_BYTES) /* most remain lenght 4 bytes */
    {
        uint8_t byte_value = buf[offset];
        remain_length_value = (remain_length_value << 7)|(byte_value&0x7F);
        
        if ((byte_value&0x80) == 0) /* the last bytes */
        {
	    LOG_DEBUG("remain_length::Find last byte [0x%02x], offset [%d], remain len bytes [%d]",
									byte_value, offset, offset + 1);
            last_byte_found = true;
            break;
        }
        
        offset++;
    }
    
    if (!last_byte_found) /* no last byte */
    {
        return ((offset >= MAX_REMAIN_LENGTH_BYTES)? FAILTURE: NEED_MORE_DATA);
    }
    
    remain_length_bytes = offset + 1;
    uint32_t pkt_total_len = remain_length_value + remain_length_bytes + 1;

    if (pkt_total_len > len)  /* 1 = fixed header len not full pkt we got */
    {
	LOG_DEBUG("pkt remain_length %d, total len %d, buf len %d", remain_length_value, pkt_total_len, len);
        return NEED_MORE_DATA;
    }
    
    return remain_length_value;
}

int CMqttPkt::read_remain_length(uint32_t &remain_length_value, uint8_t &remain_length_bytes)
{
    // Fix header has 2 bytes length
    if (left_size() < 2)
    {
        return FAILTURE;
    }
    
    int res = 0;
    if ((res = remain_length(m_buf_ptr, m_max_size,remain_length_value,remain_length_bytes)) < 0)
    {
        LOG_DEBUG("CMqttPkt::Get remain lenght failed, reason %d [-2 need more data]", res);
        return res;
    }
        
    
/*
    const int POS_START_OF_REMAIN_LENGTH = 1; // start by 1
    uint8_t *buf = m_buf_ptr + POS_START_OF_REMAIN_LENGTH;
    
    int offset = 0;
    bool last_byte_found = false;
    
    const int MAX_REMAIN_LENGTH_BYTES = 4;
    while(offset < MAX_REMAIN_LENGTH_BYTES) // most remain lenght 4 bytes
    {
        uint8_t byte_value = buf[offset];
        remain_length_value = (remain_length_value << 7)|(byte_value&0x7F);
        
        if (!has_remain_len(byte_value)) // the last bytes
        {
            last_byte_found = true;
            break;
        }
        
        offset++;
    }
    
    if (!last_byte_found) // no last byte
    {
        return ((offset >= MAX_REMAIN_LENGTH_BYTES)? FAILTURE: NEED_MORE_DATA);
    }
    
    remain_length_bytes = offset + 1;
    
    if ((remain_length_value + remain_length_bytes + 1) > m_max_size)  // 1 = fixed header len not full pkt we got
    {
        return NEED_MORE_DATA;
    }
 */

    m_offset += remain_length_bytes;
    
    return SUCCESS;
}

int CMqttPkt::read_byte(uint8_t &value)
{
    if (left_size() < 1)
    {
        return FAILTURE;
    }
    
    value =  m_buf_ptr[m_offset++];
    return SUCCESS;
}

int CMqttPkt::read_byte(std::vector<uint8_t> &payload, int len)
{
    if (left_size() < len)
    {
        return FAILTURE;
    }
  
    for (int i = 0; i < len; i++)
    {
	payload.push_back(m_buf_ptr[m_offset+i]);
    }
    
    m_offset += len;
    
    return SUCCESS;
}

int CMqttPkt::read_short(uint16_t &value)
{
    if (left_size() < 2)
    {
        return FAILTURE;
    }
    
    value =  ((uint8_t)m_buf_ptr[m_offset] << 8) | ((uint8_t)m_buf_ptr[m_offset+1]&0xFF);
    
    m_offset += 2;
    
    return SUCCESS;
}

int CMqttPkt::read_string(std::string &str)
{
    uint16_t str_len;
    
    if ((this->read_short(str_len) == FAILTURE) || (left_size() < str_len))
    {
        LOG_DEBUG("Read short or str_len %d failed, offset %d", str_len, m_offset);
        return FAILTURE;
    }
    
    // LOG_DEBUG("Get str len %d, start offset %d", str_len, m_offset);
    
    str.assign((char *)(m_buf_ptr + m_offset), str_len);
    
    m_offset += str_len;
    
    return SUCCESS;
}

// ==========  write data to pkt ============

int CMqttPkt::write_remain_length(uint32_t length, uint8_t &remain_length_bytes)
{
    // const int POS_START_OF_REMAIN_LENGTH = 1; // start by 1
    
    remain_length_bytes = 0;
    
    do
    {
        uint8_t digit = length % 128;
        length = length / 128;
       
	// LOG_DEBUG("digit %d", digit);
        // if there are more digits to encode, set the top bit of this digit
	if (length > 0)
	{
	    m_buf_ptr[m_offset + remain_length_bytes] = digit | 0x80;
	}
	else
	{
	     m_buf_ptr[m_offset + remain_length_bytes] = digit & 0x7F;
	}

        remain_length_bytes++;
        
    } while ( length > 0 );

    m_offset += remain_length_bytes;
    
    return SUCCESS;
}

int CMqttPkt::write_byte(uint8_t value)
{
    if (left_size() < 1)
    {
        return FAILTURE;
    }
    
    m_buf_ptr[m_offset++] = value;
    
    return SUCCESS;
}

int CMqttPkt::write_byte(uint8_t *buf, int size)
{
    if (left_size() < size)
    {
        return FAILTURE;
    }
    
    memcpy(m_buf_ptr + m_offset, buf, size);
    
    m_offset += size;
    
    return SUCCESS;
}

int CMqttPkt::write_short(uint16_t value)
{
    if (left_size() < 2)
    {
        return FAILTURE;
    }
    
    m_buf_ptr[m_offset++] = (value >> 8)&0xFF;
    m_buf_ptr[m_offset++] = value&0xFF;
    
    return SUCCESS;
}

int CMqttPkt::write_string(uint8_t *buf, int size)
{
    if (write_short(size) == FAILTURE)
    {
        return FAILTURE;
    }
    
    return write_byte(buf, size);
}

#ifdef _MAIN

int main()
{
    CLoggerMgr logger("log4cplus.properties");
    
    uint8_t connect_msg[] = {
        0x10, 0x43, 0x00, 0x06, 0x4d, 0x51, 0x49, 0x73,
        0x64, 0x70, 0x03, 0xee, 0x00, 0x3c, 0x00, 0x0c,
        0x64, 0x61, 0x76, 0x61, 0x64, 0x2e, 0x64, 0x69,
        0x5f, 0x63, 0x6c, 0x69, 0x00, 0x05, 0x68, 0x65,
        0x6c, 0x6c, 0x6f, 0x00, 0x0d, 0x68, 0x65, 0x6c,
        0x6c, 0x6f, 0x5f, 0x70, 0x61, 0x79, 0x6c, 0x6f,
        0x61, 0x64, 0x00, 0x07, 0x64, 0x61, 0x76, 0x61,
        0x2e, 0x64, 0x69, 0x00, 0x08, 0x70, 0x61, 0x73,
        0x73, 0x77, 0x6f, 0x72, 0x64 };
    
    CMqttPkt mqtt_msg_pkt(connect_msg, sizeof(connect_msg));
    
    if (mqtt_msg_pkt.decode() < 0)
    {
        LOG_DEBUG("Connect msg decode failed");
        return -1;
    }
    
    mqtt_msg_pkt.print();
    
  
    /*
    uint8_t fix_header = 0;
    if (mqtt_msg_pkt.read_byte(fix_header) < 0)
    {
        printf("Get fix_header failed\n");
        return -1;
    }
    
    printf("FixHeader 0x%x\n", fix_header);
    
    uint32_t remain_length_value = 0;
    uint8_t  reamin_length_bytes = 0;
    
    if (mqtt_msg_pkt.read_remain_length(remain_length_value, reamin_length_bytes) < 0)
    {
        printf("Get remain_length failed\n");
        return -1;
    }
    
    printf("Get remain lenght %d, bytes %d\n", remain_length_value, reamin_length_bytes);
    
    std::string str_protocol_name;
    if (mqtt_msg_pkt.read_string(str_protocol_name) < 0)
    {
        printf("Get protocol name failed\n");
        return -1;
    }
    
    printf("Get protocol name [%s], lenght is %d\n", str_protocol_name.c_str(), str_protocol_name.length());
    
    uint8_t ver = 0;
    if (mqtt_msg_pkt.read_byte(ver) < 0)
    {
        printf("Get ver failed\n");
        return -1;
    }
    
    printf("ver 0x%x\n", ver);
    
    uint8_t conn_flag = 0;
    if (mqtt_msg_pkt.read_byte(conn_flag) < 0)
    {
        printf("Get conn_flag failed\n");
        return -1;
    }
    typedef union
    {
        uint8_t all;
        struct
        {
        uint8_t:1;                          
            uint8_t    clean_session:1;
            uint8_t    will:1;
            uint8_t    will_qos:2;
            uint8_t    will_retail:1;
            uint8_t    password:1;
            uint8_t    username:1;
        } bits;
    }ConnectFlag;
    
    ConnectFlag flag;
    
    flag.all = conn_flag;
    
    printf("clean_session %d, will %d, will_qos %d, will_retail %d, password %d, username %d\n",
           flag.bits.clean_session, flag.bits.will, flag.bits.will_qos, flag.bits.will_retail,
           flag.bits.password, flag.bits.username);
    
    uint16_t keep_alive = 0;
    
    if (mqtt_msg_pkt.read_short(keep_alive) < 0)
    {
        printf("Get keep alive failed\n");
        return -1;
    }
    
    printf("Get keep alive %d\n", keep_alive);
    
    std::string str_client_id;
    
    if (mqtt_msg_pkt.read_string(str_client_id) < 0)
    {
        printf("Get client id failed\n");
        return -1;
    }
    
    printf("Get client_id [%s], len %d\n",str_client_id.c_str(), str_client_id.length());
    
    std::string str_will;
    if (flag.bits.will)
    {
        mqtt_msg_pkt.read_string(str_will);
        printf("Get will topic [%s]\n", str_will.c_str());
        
        mqtt_msg_pkt.read_string(str_will);
        printf("Get will msg [%s]\n", str_will.c_str());
    }
    
    std::string str_user_name;
    if (flag.bits.username)
    {
        mqtt_msg_pkt.read_string(str_user_name);
        printf("Get user name [%s]\n", str_user_name.c_str());
    }
    
    std::string str_user_passwd;
    if (flag.bits.password)
    {
        mqtt_msg_pkt.read_string(str_user_passwd);
        printf("Get user passwd [%s]\n", str_user_passwd.c_str());
    }
     */
    
    return 0;
}

#endif

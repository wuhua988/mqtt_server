//
//  mqtt_pkt.h
//  mqtt_c++
//
//  Created by davad.di on 7/13/15.
//  Copyright (c) 2015 davad.di. All rights reserved.
//

#ifndef __mqtt_c____mqtt_pkt__
#define __mqtt_c____mqtt_pkt__

#include "reactor/define.hpp"

#define NEED_MORE_DATA  -2
#define FAILTURE         -1
#define SUCCESS         0

int remain_length(uint8_t *pkt_buf, uint32_t len, uint32_t &remain_length_value, uint8_t &remain_length_bytes);

class CMqttPkt
{
public:
    CMqttPkt(uint8_t *buf, uint32_t len)
    : m_offset(0), m_max_size(len), m_buf_ptr(buf), m_remain_length_value(0), m_remain_length_bytes(0)
    {
    }
    
    int read_remain_length(uint32_t &remain_length_value, uint8_t &remain_length_bytes);
    int read_byte(uint8_t &value);
    
    int read_byte(std::vector<uint8_t> &payload, int len);
    
    int read_short(uint16_t &value);
    int read_string(std::string &str);
    
    int write_remain_length(uint32_t length, uint8_t &remain_length_bytes);
    int write_byte(uint8_t value);
    int write_byte(uint8_t *buf, int size);
    int write_short(uint16_t value);
    int write_string(uint8_t *buf, int size);
    
    uint32_t length()
    {
        return m_offset;
    }
    
    int left_size()
    {
        return m_max_size - m_offset;
    }
    
protected:
    bool has_remain_len(uint8_t value)
    {
        return ((value & 0x80) == 0x80);
    }
    
    uint32_t m_offset;
    uint32_t m_max_size;
    uint8_t   *m_buf_ptr;
    
    uint32_t m_remain_length_value;
    uint8_t m_remain_length_bytes;
};

#endif /* defined(__mqtt_c____mqtt_pkt__) */

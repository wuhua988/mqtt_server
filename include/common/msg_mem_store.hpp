#ifndef _msg_mem_store__
#define _msg_mem_store__

#include "common/singleton.hpp"
#include "msgpack/msgpack.hpp"


class CMbuf;

class CMsgMemStore
{
public:
    CMsgMemStore(uint64_t msg_id_start = 0);
    
    uint64_t next_msg_id();
    
    uint64_t last_msg_id();
    void last_msg_id(uint64_t msg_id);
    
    int del_msg(uint64_t msg_id);
    int add_msg(uint64_t msg_id, CMbuf *buf);
    
    std::unordered_map<uint64_t, CMbuf *> & msg_db();
    
    uint32_t last_update_time();
    
    void print();
protected:
    uint64_t m_last_msg_id;
    std::unordered_map<uint64_t, CMbuf *>     m_msg_db;
    uint32_t m_last_update_time;
    
public:
    MSGPACK_DEFINE(m_last_msg_id, m_last_update_time);
    
};

#define MSG_MEM_STORE   CSingleton<CMsgMemStore>::instance()

#endif


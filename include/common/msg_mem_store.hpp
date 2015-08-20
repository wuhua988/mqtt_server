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

class CMsgStat
{
public:
    void start_stat();
    void end_stat();
    
    void inc_ack_client()
    {
        m_ack_client++;
    }
    
    CLS_VAR_NO_REF_CONST(uint64_t, msg_id);
    CLS_VAR_NO_REF_CONST(std::time_t, pub_start);
    CLS_VAR_NO_REF_CONST(std::time_t, pub_end);
    
    CLS_VAR_NO_REF_CONST(uint32_t, pub_online_num);
    CLS_VAR_NO_REF_CONST(uint32_t, pub_offline_num);
    
    CLS_VAR_NO_REF_CONST(uint32_t, min_delay);
    CLS_VAR_NO_REF_CONST(uint32_t, max_delay);
    
    CLS_VAR_NO_REF_CONST(uint32_t, ack_client);
};

/*
 // store to file
 class CMsgStatMgr
 {
 public:
 int update_msg_stat(uint64_t msg_id, CMsgStat &msg_stat);
 CMsgStat & msg_stat(uint64_t msg_id);
 
 protected:
 std::unordered_map<uint64_t, CMsgStat>     m_msg_db;
 };
 */

class CMsgStatMgr
{
    enum {MAX_STAT_MSG = 65536};
    
public:
    CMsgStat & msg_stat(uint64_t msg_id);
    void flush();
    
protected:
    CMsgStat m_msg_stat[MAX_STAT_MSG];
};

#define MSG_STAT_MGR   CSingleton<CMsgStatMgr>::instance()


#endif


#include "common/mbuf.hpp"
#include "common/msg_mem_store.hpp"
#include "common/str_tools.hpp"

#include "common/thread_record.hpp"

CMsgMemStore::CMsgMemStore(uint64_t msg_id_start)
: m_last_msg_id(msg_id_start)
{
    
}

uint64_t CMsgMemStore::next_msg_id()
{
    return ++m_last_msg_id;
}

uint64_t CMsgMemStore::last_msg_id()
{
    return m_last_msg_id;
}

void CMsgMemStore::last_msg_id(uint64_t msg_id)
{
    m_last_msg_id = msg_id;
}

int CMsgMemStore::del_msg(uint64_t msg_id)
{
    auto it = m_msg_db.find(msg_id);
    
    if (it != m_msg_db.end())
    {
        m_msg_db.erase(it);
    }
    
    return 0;
}

int CMsgMemStore::add_msg(uint64_t msg_id, CMbuf *buf)
{
    m_last_update_time = std::time(nullptr);
    if (buf != nullptr)
    {
        m_msg_db[msg_id] = buf;
    }
    
    return 0;
}

std::unordered_map<uint64_t, CMbuf *> &  CMsgMemStore::msg_db()
{
    return m_msg_db;
}

uint32_t CMsgMemStore::last_update_time()
{
    return m_last_update_time;
}

void CMsgMemStore::print()
{
    LOG_DEBUG("Last msg id %ld", this->m_last_msg_id);
    LOG_DEBUG("Last update time %d", this->m_last_update_time);
}

void CMsgStat::start_stat()
{
    m_msg_id = 0;
    m_pub_end = 0;
    m_pub_online_num = 0;
    m_pub_offline_num = 0;
    
    m_min_delay = 0;
    m_max_delay = 0;
    
    m_ack_client = 0;
    
    m_pub_start = std::time(nullptr);
}

void CMsgStat::end_stat()
{
    m_pub_end = std::time(nullptr);
}

/*
 int CMsgStatMgr::update_msg_stat(uint64_t msg_id, CMsgStat &msg_stat)
 {
 
 }
 
 int CMsgStatMgr::msg_stat(uint64_t msg_id, CMsgStat &msg_stat)
 {
 
 }
 */

CMsgStat & CMsgStatMgr::msg_stat(uint64_t msg_id)
{
    uint16_t idx = (uint16_t)(msg_id&0xFFFF);
    m_msg_stat[idx].msg_id(msg_id);
    
    return m_msg_stat[idx];
}

void CMsgStatMgr::flush()
{
    LOG_DEBUG("Enter CMsgStatMgr flush()");
    
    for (int i = 0; i < MAX_STAT_MSG; i++)
    {
        CMsgStat &msg_stat =  m_msg_stat[i];
        std::time_t cur_tm = std::time(nullptr);
        
        if ((msg_stat.msg_id() > 0) && (cur_tm - msg_stat.pub_start() > 2*60)) // 1min
        {
            //msg_id, pub_start_tm, pub_end_time, cur_time, online_client, offline_client, ack_client
            std::ostringstream oss;
            oss << msg_stat.msg_id() << ",";
            oss << str_tools::format_time(msg_stat.pub_start()) << ",";
            oss << str_tools::format_time(msg_stat.pub_end()) << ",";
            oss << str_tools::format_time(std::time(nullptr)) << ",";
            
            oss << msg_stat.pub_online_num() << ",";
            oss << msg_stat.pub_offline_num() << ",";
            
            oss << msg_stat.ack_client() << "\n";
            //LOG_DEBUG("%s", oss.str().c_str());
            MSG_RECORD->put_msg(std::move(oss.str()));
            
            msg_stat.start_stat(); // clean attribute
        }
    }
}

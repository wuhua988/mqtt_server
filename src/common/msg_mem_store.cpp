#include "common/mbuf.hpp"
#include "common/msg_mem_store.hpp"

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

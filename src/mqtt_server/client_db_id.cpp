//
//  client_id_db.cpp
//  mqtt_server
//
//  Created by davad.di on 7/27/15.
//
//


#include "mqtt_server/client_id_db.hpp"
#include "mqtt_server/persist.hpp"

namespace reactor
{    
    CMsgMemStore::CMsgMemStore(): m_last_msg_id (0)
    {
    }
    
    /*
    uint64_t CMsgMemStore::add_msg(CMbuf_ptr &msg)
    {
    
        m_msg_db[++m_last_msg_id] = msg;
        
        m_last_update_time = time(NULL);
    
        return m_last_msg_id;
    }
    */
    
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
    
    /*
    int CMsgMemStore::get_msg(uint64_t msg_id, CMbuf_ptr &msg)
    {
        auto it = m_msg_db.find(msg_id);
        if (it != m_msg_db.end())
        {
            msg = it->second;
            return 0;
        }
        
        return -1;
    }
    
    int CMsgMemStore::store(CPersist* persist)
    {
	// store last_msg_id
	uint32_t chunk_len = sizeof(m_last_msg_id);
	ERROR_RETURN(persist->write_chunk_info(CHUNK_TYPE::GLOBAL_INFO, chunk_len), -1);
	ERROR_RETURN(persist->write((uint8_t *)&m_last_msg_id, sizeof(m_last_msg_id)), -1);

        return 0;
    }
    

    int CMsgMemStore::restore(uint8_t *buf, uint32_t len)
    {
	if (len == sizeof(uint64_t))
	{
	    m_last_msg_id = *((uint64_t *)buf);
	}

        return 0;
    }
    
    
    uint32_t  CMsgMemStore::last_update_time()
    {
        return m_last_update_time;
    }
    */
    
    
    int CClientIdContext::find_client_context(std::string &client_id, CMqttClientContext_ptr &client_context)
    {
        auto it = m_client_msg.find(client_id);
        
        // CMqttClientContext_ptr
        if (it == m_client_msg.end()) // client_id is not exists
        {
            return -1;
        }
        else
        {
            client_context = it->second;
        }
        
        return 0;
        
    }
    
    int CClientIdContext::add_client_context(std::string &client_id, CMqttClientContext_ptr &mqtt_client_context)
    {
        auto it = m_client_msg.find(client_id);
        
        // CClientMsgNode_ptr
        if (it == m_client_msg.end()) // client_id is not exists
        {
            m_client_msg[client_id] = mqtt_client_context;
            
            return 0;
        }
        
        return -1;
    }
    
    int CClientIdContext::del_client_context(std::string &client_id)
    {
        auto it = m_client_msg.find(client_id);
        
        if (it != m_client_msg.end())
        {
            m_client_msg.erase(client_id);
            
            return 0;
        }
        
        LOG_DEBUG("Del client context for client id [%s], but not exists", client_id.c_str());
        
        return -1;
    }
    
    
    int CClientIdContext::store(CPersist * UNUSED(persist))
    {
        // client_id :
        // msg_id1 -> msg_id2 -> msg_id3.....
        
        return 0;
    }
    
    int CClientIdContext::restore(uint8_t *UNUSED(buf), uint32 UNUSED(len))
    {
        return 0;
    }

} // end of namespace



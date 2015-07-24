#ifndef __client_id_db_h__
#define __client_id_db_h__

#include "reactor/define.hpp"
#include "mqtt_server/mqtt_context.hpp"

namespace reactor
{

    class CMsgMemStore
    {
	public:

	    CMsgMemStore(): m_last_msg_id (0)
	{
	}

	    uint64_t add_msg(CMbuf_ptr &msg)
	    {
		m_msg_db[++m_last_msg_id] = msg;

		return m_last_msg_id;
	    }

	    uint64_t next_msg_id()
	    {
		return m_last_msg_id + 1;
	    }

	    int get_msg(uint64_t msg_id, CMbuf_ptr &msg)
	    {
		auto it = m_msg_db.find(msg_id);
		if (it != m_msg_db.end())
		{
		    msg = it->second;
		    return 0;
		}

		return -1;
	    }

	    int flush()
	    {
		// store msg to file or database
		return 0;
	    }

	protected:
	    uint64_t                                    m_last_msg_id;
	    std::unordered_map<uint64_t, CMbuf_ptr>     m_msg_db;
    };

#define MSG_MEM_STORE   CSingleton<CMsgMemStore>::instance()

class CClientIdContext
{
    public:
	int find_client_context(std::string &client_id, CMqttClientContext_ptr &client_context)
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

	int add_client_context(std::string &client_id, CMqttClientContext_ptr &mqtt_client_context)
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

	int del_client_context(std::string &client_id)
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


	int flush()
	{
	    // client_id : msg_id1 -> msg_id2 -> msg_id3.....
	    return 0;
	}

    protected:
	std::unordered_map<std::string, CMqttClientContext_ptr>    m_client_msg;
};
#define CLIENT_ID_CONTEXT  CSingleton<CClientIdContext>::instance()

} // end of namespace

#endif


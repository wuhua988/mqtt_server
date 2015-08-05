#ifndef __client_id_db_h__
#define __client_id_db_h__

#include "reactor/define.hpp"
#include "common/singleton.hpp"
#include "common/mbuf.hpp"
#include "mqtt_server/mqtt_context.hpp"

class CMbuf;

namespace reactor
{
    class CPersist;

#if 0
    class CMsgMemStore
    {
	friend class CPersist;

    public: 
        CMsgMemStore();

	 uint64_t next_msg_id();  

	 uint64_t last_msg_id();
	 void last_msg_id(uint64_t msg_id);
	
	 int add_mag(CMbuf *msg)
	 {
	     return 0;
	 }

	 int del_msg(uint64_t msg_id)
	 {
	     return 0;
	 }

	 /*
        uint64_t add_msg(CMbuf_ptr &msg);
        
        uint64_t next_msg_id();

        int get_msg(uint64_t msg_id, CMbuf_ptr &msg);
	
        
        int store(CPersist* persist);
        int restore(uint8_t *buf, uint32_t len);
        
        uint32_t  last_update_time();
	*/        

    protected:
        uint64_t                                      m_last_msg_id;
        //std::unordered_map<uint64_t, CMbuf_ptr>     m_msg_db;
        //uint32_t                                    m_last_update_time;
    };
    
#define MSG_MEM_STORE   CSingleton<CMsgMemStore>::instance()
#endif 

    class CClientIdContext
    {
	friend class CPersist;

    public:
        CClientIdContext(){}
        
        int find_client_context(std::string &client_id, CMqttClientContext_ptr &client_context);
        
        int add_client_context(std::string &client_id, CMqttClientContext_ptr &mqtt_client_context);
        int del_client_context(std::string &client_id);

        int store(CPersist* persist);
        int restore(uint8_t *UNUSED(buf), uint32 UNUSED(len));

	std::unordered_map<std::string, CMqttClientContext_ptr> & client_context()
	{
	    return m_client_msg;
	}

    protected:
        std::unordered_map<std::string, CMqttClientContext_ptr>    m_client_msg;
    };


#define CLIENT_ID_CONTEXT  CSingleton<CClientIdContext>::instance()
    
} // end of namespace

#endif


#ifndef	 _mqtt_server_topic_mgr__
#define  _mqtt_server_topic_mgr__

#include <unordered_set>
#include "common/mbuf.hpp"
#include "common/singleton.hpp"
#include "mqtt_server/mqtt_connection.hpp"
#include "mqtt_server/client_id_db.hpp"

namespace reactor
{
    class CPersist;

    typedef std::unordered_set<CMqttClientContext_ptr> CONTEXT_SET;
    
    class CTopicNode
    {
	friend class CPersist;

    public:
        CTopicNode(){}
        
        void update_retain_msg(CMbuf_ptr &retain_msg);
        void clean_retain_msg();
        
        CMbuf_ptr &retain_msg();
        
        CONTEXT_SET & client_context();
        
        int add_client(CMqttClientContext_ptr &cli_context);
        
        int del_client(CMqttClientContext_ptr &cli_context);
        
        void print();
        
    private:
        //CMqttPubMessage	m_retain_msg;
        //bool            m_has_retain_msg;
        CMbuf_ptr       m_retian_msg_buf;
        CONTEXT_SET	    m_subcriber_clients;
    };
    
    typedef std::shared_ptr<CTopicNode> CTopicNode_ptr;
    
    class CSubscriberMgr
    {
	friend class CPersist;

    public:
        CSubscriberMgr(){}
        ~CSubscriberMgr(){}
        
        int add_client_context(std::string str_topic_name, CMqttClientContext_ptr cli_context);
        int del_client_context(std::string &str_topic_name, CMqttClientContext_ptr & cli_context);
        int find_client_context(std::string &str_topic_name, CONTEXT_SET &clients_set);
        
        int publish(std::string &str_topic_name, CMbuf_ptr &mbuf, CMqttPublish &publish_msg);
        
        void print();
        // int store(CPersist* persist);
        // int restore(uint8 *UNUSED(buf), uint32_t UNUSED(len));
        
    protected:
        std::unordered_map<std::string,CTopicNode_ptr> m_topic_mgr;
    };
    
    typedef CSingleton<CSubscriberMgr> SubscriberMgr;
    
#define SUB_MGR   SubscriberMgr::instance()
    
} // end of namespace

#endif


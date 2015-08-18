#ifndef _mqtt_context_h__
#define _mqtt_context_h__

#include "common/mbuf.hpp"
#include "reactor/define.hpp"
#include "mqttc++/mqtt_msg.hpp"
#include "msgpack/msgpack.hpp"

#include <list>
#include <vector>
#include <algorithm>

//#include "mqtt_server/mqtt_connection.hpp"

namespace reactor // later --> mqtt_server
{
    class CMqttConnection;
    
    class CMqttPubMessage
    {
        uint8_t m_msg_cmd;
        uint8_t m_qos;
        uint8_t m_dup_flag;
        uint8_t m_retain;
        
        uint32_t m_remain_length;               // remain length
        
        uint16_t m_msg_id;                      // src msg id
        uint16_t m_reserved;
        
        std::string m_str_topic;
        std::string m_payload;
        
        uint32_t m_db_id;                      //     time_t      m_timestamp;
    };
    
    class CMqttConnection;
    
    enum class ClientStatus {
        NONE = 0,
        CS_NEW = 1,
        CS_CONNECTED = 2,
        CS_DISCONNECTED = 3,
        CS_EXPIRING = 4
    };
    
    
    class CTMqttClientContext
    {
    public:
        CTMqttClientContext(CMqttConnection *mqtt_connection) : m_client_status(ClientStatus::CS_NEW)
        {
            m_mqtt_connection = mqtt_connection;
        }
        
        ~CTMqttClientContext()
        {
            LOG_TRACE_METHOD(__func__);
        }
        
        CMqttConnection * mqtt_connection()
        {
            return m_mqtt_connection;
        }
        
        void mqtt_connection(CMqttConnection *connection)
        {
            m_mqtt_connection = connection;
        }
        
        std::string & client_id()
        {
            return m_client_id;
        }
        
        void client_id(std::string & client_id)
        {
            m_client_id = client_id;
        }
        
        ClientStatus client_status()
        {
            return m_client_status;
        }
        
        void  client_status(ClientStatus client_status)
        {
            m_client_status = client_status;
        }
        
        bool clean_session()
        {
            return m_clean_session;
        }
        
        void init(CMqttConnect &con_msg)
        {
            m_client_status = ClientStatus::CS_CONNECTED;
            
            m_protocal_name = con_msg.proto_name();
            m_protocal_ver  = con_msg.proto_version();
            
            m_client_id     = con_msg.client_id();
            
            m_user_name     = con_msg.user_name();
            m_user_password   = con_msg.password();
            
            m_clean_session = con_msg.clean_session();
            LOG_DEBUG("In CMqttClientContext::init(), clean_session %d", m_clean_session);
            // m_will_flag     = con_msg.has_will();
            // m_will_msg      = ;
        }
        
        std::vector<CTopic>  & subcribe_topics()
        {
            return m_subcribe_topics;
        }
        
        void add_subcribe_topics(std::vector<CTopic> &sub_topics)
        {
            for (auto it = sub_topics.begin(); it != sub_topics.end(); it++)
            {
                auto topic_it = find(m_subcribe_topics.begin(), m_subcribe_topics.end(), *it);
                if ( topic_it == m_subcribe_topics.end() )
                {
                    m_subcribe_topics.push_back(*it);
                }
            }
        }
        
        int add_send_msg(CMbuf_ptr &send_mbuf)
        {
            LOG_TRACE_METHOD(__func__);
            
            m_send_msg_queue.push_back(send_mbuf);
            
            return 0;
        }
        
        std::list<CMbuf_ptr> & send_msg()
        {
            return m_send_msg_queue;
        }
        
        
        int ack_msg(uint16_t msg_id)
        {
            LOG_DEBUG("Client_id %s, ack publish msg id [%d]",m_client_id.c_str(), msg_id );
            
            auto it = m_send_msg_queue.begin();
            
            if (it !=  m_send_msg_queue.end())
            {
                uint16_t buf_id = (uint16_t)(*it)->msg_id()&0xFFFF;
                
                if (buf_id == msg_id)
                {
                    LOG_DEBUG("The send msg queue header msg_id(%d) == ack_msg_id (%d) CLEAN",
                              buf_id, msg_id);
                    
                    /*write recode to db?? */
                    m_send_msg_queue.erase(it);
                }
                else
                {
                    LOG_DEBUG("The send msg queue header msg_id(%d) != ack_msg_id (%d)",
                              buf_id, msg_id);
                }
            }
            
            return 0;
        }
        
        void prepare_store()
        {
            m_send_msgs.clear();
            for (auto it = m_send_msg_queue.begin(); it != m_send_msg_queue.end(); it++)
            {
                m_send_msgs.push_back( (*it)->msg_id());
            }
        }
        
        std::vector<uint64_t> & msg_ids()
        {
            return m_send_msgs;
        }
        
        time_t last_msg_in()
        {
            return m_last_msg_in;
        }
        
        void last_msg_in(time_t time)
        {
            m_last_msg_in = time;
        }
        
        void print()
        {
            LOG_DEBUG(" Client_id [%s]", m_client_id.c_str());
            LOG_DEBUG(" Topic num [%d]", (uint32_t)m_subcribe_topics.size());
            
            uint32_t i = 0;
            for (auto it = m_subcribe_topics.begin(); it != m_subcribe_topics.end(); it++)
            {
                LOG_DEBUG("\t [%d] %s %d", ++i, it->topic_name().c_str(), it->qos());
            }
            
            LOG_DEBUG(" Send msg queue [%d]", (uint32_t) m_send_msg_queue.size());
            
            i = 0;
            for (auto it = m_send_msg_queue.begin(); it != m_send_msg_queue.end(); it++)
            {
                LOG_DEBUG("\t [%d] id %ld, len %d", ++i, (*it)->msg_id(),(*it)->length());
            }
        }
    protected:
        // socket_t    m_sock_id;
        CMqttConnection *m_mqtt_connection = nullptr;
        
        ClientStatus m_client_status;
        
        std::string m_remote_addr;                  // ip:port
        
        std::string m_protocal_name;
        uint8_t m_protocal_ver;
        
        std::string m_client_id;                    // max len 23
        std::string m_user_name;
        std::string m_user_password;
        
        uint16_t m_keep_alive_timer;
        uint16_t m_last_msg_id;
        
        uint16_t m_max_inflight_msgs;
        
        // time related
        time_t m_last_msg_in;
        time_t m_last_msg_out;
        time_t m_ping;
        time_t m_disconnect;
        time_t m_last_retry_check;
        
        uint32_t m_cli_msg_id;                              // last msg id from client
        bool m_clean_session;
        bool m_will_flag;
        
        // CMqttPubMessage m_will_msg;
        std::vector<uint64_t> m_send_msgs;         // used to store
        
        std::list<CMbuf_ptr>  m_send_msg_queue;
        std::vector<CTopic>   m_subcribe_topics;
        
        //stat related struct
        uint64_t m_qos_msg_count[3];                    // qos0/1/2
        
    public:
        MSGPACK_DEFINE(m_remote_addr, m_protocal_name, m_protocal_ver,
                       m_client_id, m_user_name, m_user_password,
                       m_keep_alive_timer, m_last_msg_id, m_max_inflight_msgs,
                       m_last_msg_in, m_last_msg_out, m_ping, m_disconnect,
                       m_cli_msg_id, m_last_retry_check,
                       m_clean_session, m_will_flag,
                       m_send_msgs, m_subcribe_topics);
    };
    
    typedef std::shared_ptr<CTMqttClientContext> CMqttClientContext_ptr;
    
} // end namespace

#endif


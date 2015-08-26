#ifndef _mqtt_context_h__
#define _mqtt_context_h__

#include "common/mbuf.hpp"
#include "common/str_tools.hpp"
#include "reactor/define.hpp"
#include "mqttc++/mqtt_msg.hpp"
#include "msgpack/msgpack.hpp"

#include "common/msg_mem_store.hpp"
#include "common/thread_record.hpp"

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
        
        // remain length
        uint32_t m_remain_length;
        
        // src msg id
        uint16_t m_msg_id;
        uint16_t m_reserved;
        
        std::string m_str_topic;
        std::string m_payload;
        
        // time_t      m_timestamp;
        uint32_t m_db_id;
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
        CTMqttClientContext(CMqttConnection *con) : m_client_status(ClientStatus::CS_NEW)
        {
            m_mqtt_connection = con;
        }
        
        ~CTMqttClientContext()
        {
            LOG_TRACE_METHOD(__func__);
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
                    m_last_pub_time = (*it)->time();
                    m_last_pub_msg_id = (*it)->msg_id();
                    m_send_msg_queue.erase(it);
                    
                    
                    CMsgStat &msg_stat = MSG_STAT_MGR->msg_stat(m_last_pub_msg_id);
                    msg_stat.inc_ack_client();
                    LOG_DEBUG("After ack publish, msg_id %ld, ack_client %d", m_last_pub_msg_id, msg_stat.ack_client());
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
            m_send_msg_ids.clear();
            for (auto it = m_send_msg_queue.begin(); it != m_send_msg_queue.end(); it++)
            {
                m_send_msg_ids.push_back( (*it)->msg_id());
            }
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
        
        void flush()
        {
            //flush connect recorde, deli by ","
            //client_id,,0,
            /*
             10.1.16.88:52480,
             2015-08-19 04:03:20, accept
             2015-08-19 04:03:20, connct
             1969-12-31 19:00:00, disconnect  -> no disconnect msg
             2015-08-19 04:03:28, close
             0,
             2015-08-19 04:03:20, last_msg_time
             1969-12-31 19:00:00, last_pub_time
             
             0,sensors/temperature|,
             */
            
            std::ostringstream oss;
            oss << m_client_id << "," << m_user_name << "," << m_clean_session << ",";
            oss << m_remote_addr << ",";
            
            // << str_tools::format_time(std::time(nullptr)) << ","; see close time
            
            oss << str_tools::format_time(m_accept_time) << ",";
            
            oss << str_tools::format_time(m_connect_time) << ",";
            oss << str_tools::format_time(m_discon_time) << ",";
            oss << str_tools::format_time(m_close_time) << ",";
            oss << m_send_msg_queue.size() << ",";
            
            oss << str_tools::format_time(m_last_msg_time) << ",";
            
            oss << str_tools::format_time(m_last_pub_time) << ",";
            oss << m_last_pub_msg_id << ",";
            
            for (auto it = m_subcribe_topics.begin(); it != m_subcribe_topics.end(); it++)
            {
                oss << it->topic_name() << "|";
            }
            
            oss << "\n";
            //LOG_DEBUG("%s", oss.str().c_str());
            CLIENT_RECORD->put_msg(std::move(oss.str()));
            
            // send to another queue.
            // type and content
            
        }
        
    protected:
        CLS_VAR_NO_REF_CONST(CMqttConnection *, mqtt_connection);
        CLS_VAR(ClientStatus, client_status);
        CLS_VAR(std::string, remote_addr);
        CLS_VAR(std::string, protocal_name);
        CLS_VAR(uint8_t, protocal_ver);
        
        // max len 23
        CLS_VAR(std::string, client_id);
        CLS_VAR(std::string, user_name);
        CLS_VAR(std::string, user_password);
        
        CLS_VAR(uint16_t, keep_alive_timer);
        CLS_VAR(uint16_t, max_inflight_msgs);
        
        // time related
        CLS_VAR(std::time_t, accept_time);
        CLS_VAR(std::time_t, connect_time);
        CLS_VAR(std::time_t, discon_time);
        CLS_VAR(std::time_t, close_time);
        CLS_VAR(std::time_t, last_pub_time);
        CLS_VAR(std::time_t, last_msg_time);
        
        CLS_VAR(uint64_t, last_pub_msg_id);
        
        CLS_VAR(bool, clean_session);
        CLS_VAR(bool, will_flag);
        
        // used to store
        CLS_VAR(std::vector<uint64_t>, send_msg_ids);
        
        CLS_VAR_REF(std::list<CMbuf_ptr>, send_msg_queue);
        CLS_VAR(std::vector<CTopic>,  subcribe_topics);
        
    public:
        MSGPACK_DEFINE(m_remote_addr,
                       m_protocal_name,
                       m_protocal_ver,
                       m_client_id,
                       m_user_name,
                       m_user_password,
                       m_keep_alive_timer,
                       m_max_inflight_msgs,
                       m_accept_time,
                       m_connect_time,
                       m_last_pub_time,
                       m_last_msg_time,
                       m_last_pub_msg_id,
                       m_clean_session,
                       m_will_flag,
                       m_send_msg_ids,
                       m_subcribe_topics);
    };
    
    typedef std::shared_ptr<CTMqttClientContext> CMqttClientContext_ptr;
    
} // end namespace

#endif


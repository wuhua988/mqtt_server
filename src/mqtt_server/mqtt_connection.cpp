#include "mqttc++/mqtt_msg.hpp"
#include "reactor/poller_epoll.hpp"
#include "mqtt_server/subscriber_mgr.hpp"
#include "mqtt_server/mqtt_connection.hpp"

#include "mqtt_server/client_id_db.hpp"
#include "common/msg_mem_store.hpp"

int write_n(int fd, char *buf, ssize_t len)
{
    int buf_len = len;
    
    while (len > 0)
    {
        ssize_t res = ::write(fd, buf, len);
        if (res < len)
        {
            if (res == 0)
            {
                continue;
            }
            
            if (res < 0)
            {
                if (EAGAIN == errno || EWOULDBLOCK == errno || EINTR == errno )
                {
                    continue;
                }
                else
                {
                    LOG_ERROR("socket(%d) send error, errno %d. %s", fd, errno, strerror(errno));
                    return -1;
                }
            }
        }
        
        len -= res;
    }
    
    return buf_len;
}

namespace reactor
{
    
    
    int CMqttConnection::handle_input(socket_t UNUSED(sock_id))
    {
        LOG_TRACE_METHOD(__func__);
        
        // maybe some data left in last read
        int read_len = ::read(this->m_sock_handle, m_recv_buffer + m_cur_buf_pos, CMqttConnection::MAX_BUF_SIZE - m_cur_buf_pos);
        
        LOG_DEBUG("handle_input read data len %d, socket [%d]", read_len, this->m_sock_handle);
        if (read_len <= 0)
        {
            LOG_INFO("peer close the socket [%d],read_len %d, %s close now", this->m_sock_handle, read_len, strerror(errno));
            // this->handle_close();  // move to epoll called
            return -1;
        }
        
        m_recv_times++;
        m_recv_bytes += read_len;
        
        m_cur_buf_pos = read_len;
        
        return this->process_mqtt(m_recv_buffer, read_len);
    }
    
    int CMqttConnection::process_mqtt(uint8_t *buf, uint32_t len)
    {
        // first check is a full pkt
        uint32_t remain_length_value = 0;
        uint8_t remain_length_bytes = 0;
        
        // -1 failed -2 need more data, > 0 remain_length_value
        uint32_t offset = 0;
        
        // split multi pkt and process each
        while (offset < len)
        {
            int remain_len = remain_length(buf + offset, len - offset, remain_length_value, remain_length_bytes);
            if (remain_len == NEED_MORE_DATA)
            {
                LOG_DEBUG("We need more data to decode pkt, maybe should copy last part data to buffer header");
                
                // | A | B | C |
                //           ^
                //           |
                //          len
                
                if (offset != 0)
                {
                    int left_size = len - offset;
                    memmove(buf, buf + offset, left_size); // memmove can deal overlap situation
                    
                    m_cur_buf_pos = left_size;
                }
                return 0;
            }
            else if (remain_len == FAILTURE) // decode failed
            {
                return -1;      // close this socket
            }
            
            uint32_t pkt_total_len = remain_len + remain_length_bytes + 1; // 1 for fixed header
            
            // move body function to a mqtt_class
            if (this->process(buf + offset, pkt_total_len, this) < 0)
            {
                return -1;    // something wrong close this socket
            }
            
            offset += pkt_total_len;
        }
        
        // all pkts have dealt, so reset buf pos
        m_cur_buf_pos = 0;
        
        return 0;
    }
    
    int CMqttConnection::handle_close(socket_t)
    {
        LOG_TRACE_METHOD(__func__);
        
        CEventHandler::close();
        
        LOG_DEBUG("In CMqttConnection::handle_close(), clean session [%d]", m_client_context->clean_session());
        if (m_client_context->clean_session())
        {
            
            //std::list<CTopic>  & subcribe_topics()
            auto topics_vector = m_client_context->subcribe_topics();
            
            for (auto it = topics_vector.begin(); it != topics_vector.end(); it++)
            {
                LOG_DEBUG("Clean session is set, clean topic [%s]", it->topic_name().c_str());
                SUB_MGR->del_client_context(it->topic_name(), m_client_context);
            }
            
            CLIENT_ID_CONTEXT->del_client_context(m_client_context->client_id());
        }
        
        if (m_client_context.get() != nullptr)
        {
            m_client_context->mqtt_connection(nullptr);
        }
        
        // #include <inttypes.h>
        // printf("uint64: %"PRIu64"\n", u64);
        LOG_INFO("Summery Client Recv times %ld, bytes %ld, Send times %ld, bytes %ld, schedule write times %ld, cancel %ld",
                 this->m_recv_times,
                 this->m_recv_bytes,
                 this->m_send_times,
                 this->m_send_bytes,
                 this->m_schedule_write_times,
                 this->m_cancel_schedule_write_times);
        
        
        delete this;
        
        return 0;
    }
    
    // --> later move to CMqttLogic class
    int CMqttConnection::process(uint8_t *buf, uint32_t len, CMqttConnection *mqtt_connection)
    {
        MqttType msg_type = (MqttType)((buf[0]&0xF0)>>4);
        
        int res = 0;
        
        switch(msg_type)
        {
            case MqttType::CONNECT:
            {
                LOG_DEBUG("Recv mqtt CONNECT msg");
                res = this->handle_connect_msg(buf, len, mqtt_connection);
                break;
            }
                
            case MqttType::PUBLISH:
            {
                LOG_DEBUG("Recv mqtt PUBLISH msg");
                res = this->handle_publish_msg(buf, len, mqtt_connection);
                break;
            }
                
            case MqttType::PUBACK:
            {
                LOG_DEBUG("Recv mqtt PUBACK msg");
                res = this->handle_puback_msg(buf, len, mqtt_connection);
                break;
            }
                
            case MqttType::SUBSCRIBE:
            {
                LOG_DEBUG("Recv mqtt SUBSCRIBE msg");
                res = this->handle_subscribe_msg(buf, len, mqtt_connection);
                break;
            }
                
            case  MqttType::UNSUBSCRIBE:
            {
                LOG_DEBUG("Recv mqtt UNSUBSCRIBE msg");
                res = this->handle_unsubscribe_msg(buf, len, mqtt_connection);
                break;
            }
                
            case MqttType::PINGREQ:
            {
                LOG_DEBUG("Recv mqtt PINGREQ msg");
                res = this->handle_pingreq_msg(buf, len, mqtt_connection);
                break;
            }
                
            case MqttType::DISCONNECT:
            {
                LOG_DEBUG("Recv mqtt DISCONNECT msg");
                res = this->handle_disconnect_msg(buf, len, mqtt_connection);
                
                res = -1; // close this socket
                break;
            }
                
            default:
            {
                res = -1;
                LOG_INFO("Undealed msg type %d", msg_type);
                break;
            }
        } // end of switch
        
        return res;
    }
    
    int CMqttConnection::handle_connect_msg(uint8_t *buf, uint32_t len, CMqttConnection *mqtt_connection)
    {
        if (mqtt_connection == nullptr)
        {
            LOG_DEBUG("mqtt_connection should not be nullptr");
            return -1;
        }
        
        int res = 0;
        CMqttConnect conn_msg(buf,len);
        
        if (conn_msg.decode() < 0)
        {
            LOG_DEBUG("Connect decode failed");
            return -1;
        }
        conn_msg.print();
        
        CMqttConnAck::Code ack_code = CMqttConnAck::Code::ACCEPTED;
        std::string proto_name = conn_msg.proto_name();
        
        uint8_t proto_version = conn_msg.proto_version();
        
        if ( (proto_version != 3) && (proto_version != 4) )
        {
            ack_code = CMqttConnAck::Code::BAD_VERSION;
        }
        else if ( ((conn_msg.proto_version() == 3) && (proto_name.compare("MQIsdp") != 0))
                 || ((conn_msg.proto_version() == 4) &&  (proto_name.compare("MQTT") != 0)) )
        {
            ack_code = CMqttConnAck::Code::BAD_VERSION;
        }
        
        //if (user_name and passwd)
        //{
        //      ack_code = CMqttConnAck::Code::BAD_USER_OR_PWD; or NO_AUTH
        //}
        
        // bool send_offline_flag = false;
        
        std::string client_id = conn_msg.client_id();
        if ( client_id.empty() )
        {
            ack_code = CMqttConnAck::Code::BAD_ID;
        }
        else
        {
            CMqttClientContext_ptr msg_cli_context;
            if ( CLIENT_ID_CONTEXT->find_client_context(client_id,msg_cli_context) != -1)
            {
                LOG_DEBUG("Find last client context for client_id [%s]", client_id.c_str());
                CMqttConnection *last_mqtt_connection = msg_cli_context->mqtt_connection();
                if (last_mqtt_connection != nullptr)
                {
                    last_mqtt_connection->handle_close(INVALID_SOCKET);
                }
                
                msg_cli_context->init(conn_msg);
                
                msg_cli_context->mqtt_connection(this);
                mqtt_connection->client_context(msg_cli_context);
                
                // send_offline_flag = true;
            }
            else
            {
                LOG_DEBUG("Cann't Find last client context for client_id [%s]", client_id.c_str());
                
                // client_context() fuction can handle nullptr situation
                CMqttClientContext_ptr &cli_context = mqtt_connection->client_context();
                cli_context->init(conn_msg);
                
                // add client_context to
                CLIENT_ID_CONTEXT->add_client_context(client_id, cli_context);
            }
        }
        
        if ( ack_code != CMqttConnAck::Code::ACCEPTED )
        {
            res = -1;
        }
        
        CMbuf_ptr mbuf = make_shared<CMbuf>(32); // 32 enough for connack
        CMqttConnAck con_ack(mbuf->write_ptr(),mbuf->max_size(), ack_code);
        
        int enc_len = 0;
        if ((enc_len = con_ack.encode()) < 0)
        {
            res = -1;
        }
        else
        {
            mbuf->write_ptr(enc_len);
            if (mqtt_connection != nullptr)
            {
                res = mqtt_connection->put(mbuf);
            }
        }
        
        // send offline msg
        CMqttClientContext_ptr &cli_context = mqtt_connection->client_context();
        std::list<CMbuf_ptr> &offline_msg = cli_context->send_msg();
        
        int count = 0;
        for (auto it = offline_msg.begin(); it != offline_msg.end(); it++)
        {
            if ( (res = this->put(*it)) == -1)
            {
                LOG_DEBUG("Send offline msg failed to client id [%s]", client_id.c_str());
                break;
            }
            
            count++;
        }
        
        LOG_DEBUG("Send offline [%d] msg to client id [%s]", count, client_id.c_str());
        
        return res;
    }
    
    int CMqttConnection::handle_publish_msg(uint8_t *buf, uint32_t len, CMqttConnection *mqtt_connection)
    {
        if (mqtt_connection == nullptr)
        {
            LOG_DEBUG("mqtt_connection should not be nullptr");
            return -1;
        }
        
        CMqttClientContext_ptr &cli_context = mqtt_connection->client_context();
        if (cli_context->client_status() != ClientStatus::CS_CONNECTED)
        {
            LOG_INFO("client is not wiht CONNECT msg");
            return -1;
        }
        
        CMqttPublish publish(buf,len);
        if (publish.decode() < 0)
        {
            LOG_INFO("publish decode failed");
            return -1;
        }
        
        publish.print();
        
        // 1. first send ack to sender
        CMbuf_ptr mbuf_pub_ack = make_shared<CMbuf>(64);
        CMqttPublishAck  pub_ack(mbuf_pub_ack->write_ptr(),mbuf_pub_ack->max_size(),publish.msg_id());
        
        int res = 0;
        int enc_len = 0;
        
        if ((enc_len = pub_ack.encode()) < 0)
        {
            res = -1;
        }
        else
        {
            mbuf_pub_ack->write_ptr(enc_len);
            if (mqtt_connection != nullptr)
            {
                res = mqtt_connection->put(mbuf_pub_ack);
            }
        }
        
        // 1. change msg id and store msg
        CMbuf_ptr mbuf_pub  = make_shared<CMbuf>(len);   // copy buf and send to other client
        
        uint64_t publish_msg_id = MSG_MEM_STORE->next_msg_id();   // change msg_id to msg_id
        mbuf_pub->msg_id(publish_msg_id);
        
        uint32_t msg_id_offset = publish.msg_id_offset();
        if (msg_id_offset < len)
        {
            uint16_t msg_id = (uint16_t)publish_msg_id&0xFFFF;
            
            buf[msg_id_offset] = (msg_id >> 8)&0xFF;
            buf[msg_id_offset+1] = msg_id&0xFF;
            
            LOG_DEBUG("Change publish msg_id to [%d]", msg_id);
        }
        
        mbuf_pub->copy(buf, len);  // change buf msg_id
        
        
        // 2. publish to clients
        uint32_t start_tm = time(0);
        int pub_count = SUB_MGR->publish(publish.topic_name(), mbuf_pub, publish);
        
        uint32_t diff = time(0) - start_tm;
        
        LOG_ERROR("This publish cost %d (s) to %d clients", diff, pub_count);
        
        return res;
    }
    
    int CMqttConnection::handle_puback_msg(uint8_t *buf, uint32_t len, CMqttConnection *mqtt_connection)
    {
        if (mqtt_connection == nullptr)
        {
            LOG_DEBUG("mqtt_connection should not be nullptr");
            return -1;
        }
        
        CMqttPublishAck pub_ack(buf, len);
        if (pub_ack.decode() < 0)
        {
            LOG_DEBUG("CMqttPublishAck decode failed.");
            return -1;
        }
        
        CMqttClientContext_ptr &cli_context = mqtt_connection->client_context();
        cli_context->ack_msg(pub_ack.msg_id());
        
        return 0;
    }
    
    int  CMqttConnection::handle_subscribe_msg(uint8_t *buf, uint32_t len, CMqttConnection *mqtt_connection)
    {
        if (mqtt_connection == nullptr)
        {
            LOG_DEBUG("mqtt_connection should not be nullptr");
            return -1;
        }
        
        CMqttClientContext_ptr &cli_context = mqtt_connection->client_context();
        if (cli_context->client_status() != ClientStatus::CS_CONNECTED)
        {
            LOG_INFO("client is not wiht CONNECT msg");
            return -1;
        }
        
        CMqttSubscribe sub(buf,len);
        if (sub.decode() < 0)
        {
            LOG_INFO("Sub decode failed");
            return -1;
        }
        
        // add to client_context
        cli_context->add_subcribe_topics(sub.sub_topics());
        
        sub.print();
        
        // 1. send sub ack
        CMbuf_ptr mbuf_sub_ack = make_shared<CMbuf>(64);
        CMqttSubAck sub_ack(mbuf_sub_ack->write_ptr(),mbuf_sub_ack->max_size(), sub.msg_id(), sub.topics_qos());
        
        int res = 0;
        int enc_len = 0;
        
        if ((enc_len = sub_ack.encode()) < 0)
        {
            res = -1;
        }
        else
        {
            mbuf_sub_ack->write_ptr(enc_len);
            if (mqtt_connection != nullptr)
            {
                res = mqtt_connection->put(mbuf_sub_ack);
            }
        }
        
        // 2. add cli_context to topic_name and send retain msg
        std::vector<std::string> topics = sub.topics_name();
        for (auto it = topics.begin(); it != topics.end(); it++)
        {
            SUB_MGR->add_client_context(*it, mqtt_connection->client_context());
        }
        
        // change for performance
        if (log4cplus::Logger::getRoot().isEnabledFor(log4cplus::DEBUG_LOG_LEVEL))
        {
            SUB_MGR->print();
        }
        
        return res;
    }
    
    int  CMqttConnection::handle_unsubscribe_msg(uint8_t *buf, uint32_t len, CMqttConnection *mqtt_connection)
    {
        if (mqtt_connection == nullptr)
        {
            LOG_DEBUG("mqtt_connection should not be nullptr");
            return -1;
        }
        
        CMqttClientContext_ptr &cli_context = mqtt_connection->client_context();
        if (cli_context->client_status() != ClientStatus::CS_CONNECTED)
        {
            LOG_INFO("client is not wiht CONNECT msg");
            return -1;
        }
        
        CMqttUnsubscribe un_sub(buf,len);
        if (un_sub.decode() < 0)
        {
            LOG_INFO("UNSub decode failed");
            return -1;
        }
        
        un_sub.print();
        
        // delete topic from sub_mgr
        // add cli_context to topic_name
        std::vector<std::string> topics = un_sub.topics_name();
        for (auto it = topics.begin(); it != topics.end(); it++)
        {
            SUB_MGR->del_client_context(*it, mqtt_connection->client_context());
        }
        
        CMbuf_ptr mbuf_unsub_ack = make_shared<CMbuf>(64);
        
        // CMqttFixedHeader fixed_header(MqttType::UNSUBACK);
        // CMqttUnsubAck unsub_ack(mbuf_unsub_ack->write_ptr(), mbuf_unsub_ack.max_size(), fixed_header, un_sub.msg_id());
        
        CMqttUnsubAck unsub_ack(mbuf_unsub_ack->write_ptr(), mbuf_unsub_ack->max_size(), un_sub.msg_id());
        
        int res = 0;
        int enc_len = 0;
        
        if ((enc_len = unsub_ack.encode()) < 0)
        {
            res = -1;
        }
        else
        {
            mbuf_unsub_ack->write_ptr(enc_len);
            if (mqtt_connection != nullptr)
            {
                res = mqtt_connection->put(mbuf_unsub_ack);
            }
        }
        
        return res;
    }
    
    int  CMqttConnection::handle_pingreq_msg(uint8_t *UNUSED(buf), uint32_t UNUSED(len), CMqttConnection *mqtt_connection)
    {
        if (mqtt_connection == nullptr)
        {
            LOG_DEBUG("mqtt_connection should not be nullptr");
            return -1;
        }
        
        CMbuf_ptr mbuf_pingresp = make_shared<CMbuf>(4);
        
        // CMqttFixedHeader fixed_header(MqttType::PINGRESP);
        // CMqttPingResp ping_rsp(mbuf_pingresp->write_ptr(), mbuf_pingresp->max_size(), fixed_header);
        CMqttPingResp ping_rsp(mbuf_pingresp->write_ptr(), mbuf_pingresp->max_size());
        
        int res = 0;
        int enc_len = 0;
        
        if ((enc_len = ping_rsp.encode()) < 0)
        {
            res = -1;
        }
        else
        {
            mbuf_pingresp->write_ptr(enc_len);
            if (mqtt_connection != nullptr)
            {
                res = mqtt_connection->put(mbuf_pingresp);
            }
        }
        
        return res;
    }
    
    int CMqttConnection::handle_disconnect_msg(uint8_t *UNUSED(buf), uint32_t UNUSED(len), CMqttConnection *mqtt_connection)
    {
        if (mqtt_connection == nullptr)
        {
            LOG_DEBUG("mqtt_connection should not be nullptr");
            return -1;
        }
        
        CMqttClientContext_ptr &cli_context = mqtt_connection->client_context();
        cli_context->client_status(ClientStatus::CS_DISCONNECTED);
        
        if ( cli_context->clean_session())
        {
            // TODOLIST:
            // deal client id msg from db msg
        }
        return 0;
    }
    
} // end of namespace


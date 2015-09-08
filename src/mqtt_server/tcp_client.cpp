#include "mqtt_server/tcp_client.hpp"
#include "mqttc++/mqtt_msg.hpp"
#include "reactor/poller.hpp"

namespace reactor
{
    int TCPClient::open(void *data)
    {
        m_client_socket = INVALID_SOCKET;
        
        m_server_addr = *(CSockAddress *)data;
        
        // set non-block ?
        m_thread_handler = std::thread(&TCPClient::svc, this);
        
        return 0;
    }
    
    int TCPClient::connect()
    {
        LOG_DEBUG("Enter TCPClient connect()");
        
        if (m_client_socket != INVALID_SOCKET)
        {
            // send_disconnct msg
            this->send_disconnect_msg();
            ::close(m_client_socket);
            
            m_client_socket = INVALID_SOCKET;
        }
        
        m_client_socket = socket(AF_INET,SOCK_STREAM,0);
        LOG_DEBUG("MQTTClient_socket %d", m_client_socket);
        
        if( m_client_socket < 0)
        {
            LOG_ERROR("Create socket failed, errno %d, %s", errno, strerror(errno));
            return -1;
        }
        
        struct timeval tm = {10,0}; // 10 s
        // setsockopt(socket，SOL_SOCKET,SO_SNDTIMEO，(char *)&timeout,sizeof(struct timeval));
        setsockopt(m_client_socket,SOL_SOCKET,SO_RCVTIMEO,(const void *)&tm, (socklen_t)sizeof(struct timeval));
        
        
        struct sockaddr_in server_addr = m_server_addr.sock_addr();
        socklen_t server_addr_length = sizeof(server_addr);
        
        uint32_t timeout = 1;
        while( ::connect(m_client_socket,(struct sockaddr*)&server_addr, server_addr_length) < 0)
        {
            if (!m_running_flag)
            {
                return -1;
            }
            
            LOG_ERROR("Connect to %s:%d failed! error %d, %s",
                      m_server_addr.get_ip().c_str(), m_server_addr.get_port(), errno, strerror(errno));
		          
            ::sleep(timeout);
            timeout *= 2;
            
            if (timeout >= 32)
            {
                timeout = 1;
            }
        }
        
        LOG_DEBUG("Mqtt Client connect to %s:%d succeed. Ready to send connect msg",
                  m_server_addr.get_ip().c_str(), m_server_addr.get_port());
        
        // send connect msg
        this->send_connect_msg();
        
        m_cur_buf_pos = 0;
        return 0;
    }
    
    void TCPClient::svc()
    {
        this->connect();
        
        uint32_t last_msg_time = std::time(nullptr);
        uint32_t send_ping_req_time = 0;
        
        m_cur_buf_pos = 0;
        
        while(m_running_flag)
        {
            uint32_t cur_time = std::time(nullptr);
            if ((send_ping_req_time > 5) || (cur_time - last_msg_time  > 5*60))// 5min, we not recv msg, reconnect
            {
                // timeout reconect
                send_ping_req_time = 0;
                LOG_DEBUG("Send ping req more than five or timeout more than 5min");
                if (this->connect() < 0)
                {
                    LOG_DEBUG("Connect() return -1. Something wrong. so break");
                    break;
                }
            }
            if (m_cur_buf_pos >= MAX_BUF_SIZE)
            {
                LOG_DEBUG("Mqtt Client Buf cur pos(%d) > max_buf (%d) ", m_cur_buf_pos, MAX_BUF_SIZE);
                break;
            }
            
            int read_len = ::read(this->m_client_socket, m_recv_buffer + m_cur_buf_pos, MAX_BUF_SIZE - m_cur_buf_pos);
            if (read_len <= 0)
            {
                int my_errno = errno;
                if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
                {
                    LOG_DEBUG("Read len <= 0, errno %d, %s, send req time %d",
                              errno, strerror(errno), send_ping_req_time);
                    // timeout
                    // send pingreq
                    this->send_pingreq_msg();
                    send_ping_req_time++;
                    continue;
                }
                else
                {
                    LOG_DEBUG("Read len -1, errno %d, %s", my_errno, strerror(my_errno));
                    
                    // error occurs, so should reconnect
                    if ( this->connect() < 0)
                    {
                        LOG_DEBUG("Connect() return -1. Something wrong. so break");
                        break;
                    }
                    
                    continue;
                }
            }
            
            send_ping_req_time = 0;
            last_msg_time = std::time(nullptr); // current
            
            LOG_DEBUG("MqttClient: handle_input read data len %d, socket [%d]", read_len, this->m_client_socket);
            m_cur_buf_pos += read_len;
            
            int res = this->process(m_recv_buffer,m_cur_buf_pos);
            if (res == NEED_MORE_DATA)
            {
                LOG_DEBUG("MqttClient: need more data");
                continue; // read again;
            }
            if (res == -1)
            {
                LOG_DEBUG("Client process mqtt msg return -1");
                if (this->connect() < 0)
                {
                    LOG_DEBUG("Connect() return -1. Something wrong. so break");
                    break;
                }
            }
        }
        
        this->send_disconnect_msg();
        ::close(this->m_client_socket);
        
        LOG_INFO("Thread svc reached the end. Exit now!!! ");
        return;
    }
    
    int TCPClient::process(uint8_t *buf, uint32_t len)
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
                LOG_DEBUG("Get reamin length failed");
                return -1; // close this socket
            }
            
            uint32_t pkt_total_len = remain_len + remain_length_bytes + 1; // 1 for fixed header
            
            // move body function to a mqtt_class
            if (this->process_mqtt(buf + offset, pkt_total_len) < 0)
            {
                return -1; // something wrong close this socket
            }
            
            offset += pkt_total_len;
        }
        
        // all pkts have dealt, so reset buf pos
        m_cur_buf_pos = 0;
        
        return 0;
    }
    
    
    int TCPClient::process_mqtt(uint8_t *buf, uint32_t len)
    {
        LOG_DEBUG("In Proess mqtt, buf len %d", len);
        
        MqttType msg_type = (MqttType)((buf[0]&0xF0)>>4);
        int res = 0;
        
        switch(msg_type)
        {
            case MqttType::CONNACK:
            {
                LOG_DEBUG("Recv mqtt CONNEACK msg");
                
                uint16_t msg_id = 1;
                this->send_subscribe_msg(msg_id);
                break;
            }
                
            case MqttType::PUBLISH:
            {
                LOG_DEBUG("Recv mqtt PUBLISH msg");
                CMqttPublish publish(buf,len);
                if (publish.decode() < 0)
                {
                    LOG_INFO("publish decode failed");
                    return -1;
                }
                
                // notify to another server
                CMbuf_ptr mbuf = make_shared<CMbuf>(len);
                mbuf->copy(buf, len);
                mbuf->msg_id(publish.msg_id());
                mbuf->msg_type(MSG_PUBLISH);
                m_reactor->notify(mbuf);
                
                this->send_pub_ack_msg(publish.msg_id());
                
                break;
            }
                
                
            case MqttType::SUBACK:
            {
                LOG_DEBUG("Recv mqtt SUBACK msg");
                break;
            }
                
            case MqttType::PINGRESP:
            {
                LOG_DEBUG("Recv mqtt PINGRESP msg");
                break;
            }
                
            default:
            {
                res = -1;
                LOG_INFO("MqttClient: Undealed msg type %d", msg_type);
                break;
            }
        }
        // end of switch
        
        return res;
    }
    
    int TCPClient::write_n(uint8_t *buf, uint32_t len)
    {
        while (len > 0)
        {
            ssize_t res = write(m_client_socket, buf, len);
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
                        LOG_ERROR("socket(%d) send error, errno %d.", m_client_socket, errno);
                        return -1;
                    }
                }
            }
            
            len -= res;
        }
        
        return 0;
    }
    
    // -1 error, 0 timeout (or intertunpt or ewouled block), len > 0 read succeed
    int TCPClient::read(uint8_t *buf, uint32_t len, uint32_t tm)                                 // timeout ms
    {
        fd_set set;
        struct timeval timeout;
        int res = -1;
        
        FD_ZERO(&set); /* clear the set */
        FD_SET(m_client_socket, &set); /* add our file descriptor to the set */
        
        timeout.tv_sec = tm/1000;
        timeout.tv_usec = (tm%1000)*1000;
        
        res = select(m_client_socket + 1, &set, NULL, NULL, &timeout);
        if(res == -1)
        {
            LOG_ERROR("Select error %d, %s", errno, strerror(errno)); /* an error accured */
            return -1;
        }
        else if(res == 0)
        {
            LOG_DEBUG("Select timeout"); /* a timeout occured */
            return 0;
        }
        else
        {
            res = ::read(m_client_socket, buf, len); /* there was data to read */
            if (res <= 0)
            {
                if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
                {
                    return 0;
                }
                
                LOG_INFO("peer close the socket [%d],read_len %d, %s close now",
                         this->m_client_socket, res, strerror(errno));
                
                return -1;
            }
        }
        
        return res;
    }
    
    int TCPClient::send_connect_msg()
    {
        uint8_t buf[256];
        CMqttConnect connect_msg(buf, 256, MqttType::CONNECT);
        connect_msg.client_id(m_str_client_id);
        connect_msg.clean_session(false);
        
        int pkt_len = connect_msg.encode();
        if (pkt_len <= 0)
        {
            LOG_DEBUG("Connect msg encode failed");
            return -1;
        }
        
        return this->write_n(buf, pkt_len);
    }
    
    int TCPClient::send_pingreq_msg()
    {
        uint8_t buf[16];
        CMqttPingReq ping_req(buf, 16);
        
        int pkt_len = ping_req.encode();
        if (pkt_len <= 0)
        {
            LOG_DEBUG("CMqtt ping req encode failed");
            return -1;
        }
        
        return this->write_n(buf, pkt_len);;
    }
    
    int TCPClient::send_disconnect_msg()
    {
        uint8_t buf[16];
        CMqttDisconnect disconnect_msg(buf, 16);
        
        int pkt_len = disconnect_msg.encode();
        if (pkt_len <= 0)
        {
            LOG_DEBUG("CMqtt disconnect msg encode failed");
            return -1;
        }
        
        return this->write_n(buf, pkt_len);
    }
    
    int TCPClient::send_subscribe_msg(uint16_t msg_id)
    {
        uint8_t buf[256];
        
        m_topic_qos = 1;
        CMqttSubscribe sub_msg(buf, 256, msg_id, m_str_topic_name, m_topic_qos);
        
        int pkt_len = sub_msg.encode();
        if (pkt_len <= 0)
        {
            LOG_DEBUG("Subscribe encode faield");
            return -1;
        }
        
        return this->write_n(buf, pkt_len);
    }
    
    int TCPClient::send_pub_ack_msg(uint16_t msg_id)
    {
        uint8_t buf[16];
        CMqttPublishAck pub_ack(buf, 16, msg_id);
        
        int pkt_len = pub_ack.encode();
        if (pkt_len <= 0)
        {
            LOG_DEBUG("Publish ack ecnode failed");
            return -1;
        }
        
        return this->write_n(buf, pkt_len);
    }
}


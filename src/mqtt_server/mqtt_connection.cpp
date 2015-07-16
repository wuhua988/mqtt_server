#include "mqtt_server/mqtt_connection.hpp" 
#include "mqttc++/mqtt_msg.hpp"

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
        
        char *buf = NULL;
        
        if (this->m_recv_mbuf.get() == nullptr)
        {
            LOG_DEBUG("m_recv_buf ptr is null , allocate new one");
            m_recv_mbuf = make_shared<CMbuf>(16384);
        }
        
        if (this->m_recv_mbuf.get() == nullptr)
        {
            LOG_ERROR("Get mbuf failed. pointer is null");
            return -1;
        }
        
        buf = (char *)m_recv_mbuf->write_ptr();
        int max_size = m_recv_mbuf->available_buf();
        
        
        int read_len = ::read(this->m_sock_handle, buf, max_size);
        
        LOG_DEBUG("handle_input read data len %d, socket [%d]", read_len, this->m_sock_handle);
        if (read_len <= 0)
        {
            LOG_INFO("peer close the socket [%d],read_len %d, %s close now", this->m_sock_handle, read_len, strerror(errno));
            // this->handle_close();  // move to epoll called
            return -1;
        }
        
        m_recv_mbuf->write_ptr(read_len);
        
        m_recv_times++;
        m_recv_bytes += read_len;
        
        return this->process_mqtt(m_recv_mbuf);
        
    }
    
    int CMqttConnection::handle_close(socket_t)
    {
        LOG_TRACE_METHOD(__func__);
        CEventHandler::close();

	// check clean session flag
	m_client_context->mqtt_connection(nullptr);

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
    
    int CMqttConnection::process_http(CMbuf_ptr &mbuf)
    {
        const char *http_header = "HTTP/1.1 200 ok \r\nContent-Length:5\r\n\r\nHello";
        const int http_header_len = strlen(http_header);
        
        // we don't care read content
        mbuf->reset();
        mbuf->copy((uint8_t *)http_header, http_header_len);
        
        return this->put(mbuf);
    }
    
    int CMqttConnection::process_echo(CMbuf_ptr &mbuf)
    {
        return this->put(mbuf);
    }
    
    int CMqttConnection::process_echo()
    {
        char buf[16384];
        int max_size = 16384;
        int read_len = ::read(this->m_sock_handle, buf, max_size);
        
        if (read_len <= 0)
        {
            return -1;
        }
        
        return ::write_n(this->m_sock_handle, buf, read_len);
    }
    
    int CMqttConnection::process_mqtt(CMbuf_ptr &mbuf)
    {
	// first check is a full pkt
	uint32_t remain_length_value = 0;
	uint8_t remain_length_bytes = 0;
	
	// -1 failed -2 need more data, > 0 remain_length_value
	int res = remain_length(mbuf->read_ptr(), mbuf->length(), 
				    remain_length_value, remain_length_bytes);

	if (res == NEED_MORE_DATA)
	{
	    LOG_INFO("We need more data");
	    return 0;
	}
	else if (res == FAILTURE)
	{
	    return -1;
	}

	// full pkt here
	uint8_t *pread = mbuf->read_ptr();
	uint32_t length = mbuf->length();
	MqttType msg_type = (MqttType)((pread[0]&0xF0)>>4);

	res = 0;

	int enc_len = 0; 

	switch(msg_type)
	{
	    case MqttType::CONNECT:
		{
		    CMqttConnect conn(pread,length);
		    if (conn.decode() < 0)
		    {
			LOG_INFO("Connect decode failed");
			res = -1;

			break;
		    }

		    conn.print();
		    
		    m_client_context = make_shared<CTMqttClientContext>();
		    
		    m_client_context->mqtt_connection(this);
		    m_client_context->client_id(conn.client_id());

		    mbuf->reset();
		    length = mbuf->max_size();

		    CMqttFixedHeader fixed_header(MqttType::CONNACK);
		    CMqttConnAck con_ack(pread,length,fixed_header, CMqttConnAck::Code::ACCEPTED);
		    
		    enc_len = 0;
		    if ((enc_len = con_ack.encode()) < 0)
		    {
			res = -1;
		    }
		    else
		    {
			mbuf->write_ptr(enc_len);
		    }

		    break;
		}

	    case MqttType::PUBLISH:
		{
		    CMqttPublish publish(pread,length);
		    if (publish.decode() < 0)
		    {
			LOG_INFO("publish decode failed");
			res = -1;
			break;
		    }
		    
		    publish.print();

		    // copy buf and send to other client
		    CMbuf_ptr mbuf_publish = mbuf;
		    CONTEXT_SET client_context_set;

		    if (SUB_MGR->find_client_context(publish.topic_name(), client_context_set) != -1)
		    {
			for (auto it = client_context_set.begin(); it != client_context_set.end(); it++)
			{
			    auto mqtt_conn = (*it)->mqtt_connection();
			    if (mqtt_conn != nullptr)
			    {
				mqtt_conn->put(mbuf_publish);
			    }
			}
		    }


		    mbuf = make_shared<CMbuf>(64);
		    pread = mbuf->read_ptr();
		    length  = mbuf->max_size();

		    // mbuf->reset();
		    CMqttFixedHeader fixed_header(MqttType::PUBACK);
		    CMqttPublishAck  pub_ack(pread,length,fixed_header, publish.msg_id());
		    if ((enc_len = pub_ack.encode()) < 0)
		    {
			res = -1;
		    }
		    else
		    {
			mbuf->write_ptr(enc_len); 
		    }
    
		    break;
		}

	    case MqttType::PUBACK:
		{
		    return 0;
		    break;
		}

	    case MqttType::SUBSCRIBE:
		{
		    CMqttSubscribe sub(pread,length);
		    if (sub.decode() < 0)
		    {
			LOG_INFO("Sub decode failed");
			res = -1;
		    }

		    sub.print();   
		    
		    // add cli_context to topic_name
		    std::vector<std::string> topics = sub.topics_name(); 
		    for (auto it = topics.begin(); it != topics.end(); it++)
		    {
			SUB_MGR->add_client_context(*it, m_client_context);
		    }

		    SUB_MGR->print();

		    mbuf->reset();
		    length = mbuf->max_size();

		    CMqttFixedHeader fixed_header(MqttType::SUBACK);
		    CMqttSubAck sub_ack(pread,length, fixed_header, sub.msg_id(), sub.topics_qos());

		    if ((enc_len = sub_ack.encode()) < 0)
		    {
			res = -1;
		    }
		    else
		    {
			mbuf->write_ptr(enc_len);  
		    }

		    break;
		}

	    case  MqttType::UNSUBSCRIBE:
		{
		    CMqttUnsubscribe un_sub(pread,length);
		    if (un_sub.decode() < 0)
		    {
			LOG_INFO("UNSub decode failed");
			res = -1;
		    }

		    un_sub.decode();

		    mbuf->reset();
		    length = mbuf->max_size();
		    CMqttFixedHeader fixed_header(MqttType::UNSUBACK);
		    
		    CMqttUnsubAck unsub_ack(pread, length, fixed_header, un_sub.msg_id());
		    if ((enc_len = unsub_ack.encode()) < 0)
		    {
			res = -1;
		    }
		    else
		    {
			mbuf->write_ptr(enc_len);
		    }

		    break;
		}

	    case MqttType::PINGREQ:
		{
		    mbuf->reset(); 
		    
		    length = mbuf->max_size();

		    CMqttFixedHeader fixed_header(MqttType::PINGRESP); 
		    CMqttPingResp ping_rsp(pread, length, fixed_header);
		    if ((enc_len = ping_rsp.encode()) < 0) 
		    {
			res = -1;
		    }
		    else
		    {
			mbuf->write_ptr(enc_len);  
		    }

		    break;
		}

	    case MqttType::DISCONNECT:
		{
		    res = -1;
		    break;
		}

	    default:
		{
		    res = -1;
		    LOG_INFO("Undealed msg type %d", msg_type);
		    break;
		}
	}
	
	if (res == -1)
	{
	    mbuf->reset();
	    return -1;
	}

	CMbuf_ptr mbuf_copy = mbuf;
	m_recv_mbuf = make_shared<CMbuf>(16384);
	return this->put(mbuf_copy); 
    }

} // end of namespace


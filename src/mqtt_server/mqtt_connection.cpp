#include "mqtt_server/mqtt_connection.hpp" 

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
    
    int CMqttConnection::process_mqtt(CMbuf_ptr &)
    {
        return 0;
    }

} // end of namespace


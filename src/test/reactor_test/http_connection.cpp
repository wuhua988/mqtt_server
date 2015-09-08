#include "http_connection.hpp"

#include "common/str_tools.hpp"
#include "common/mbuf.hpp"

#include "mqttc++/mqtt_msg.hpp"
#include "common/msg_mem_store.hpp"
#include "reactor/poller.hpp"

namespace http
{
    int HttpConnection::handle_input(socket_t)
    {
        LOG_TRACE_METHOD(__func__);
        
        int read_len = ::read(this->m_sock_handle, m_recv_buffer + m_cur_buf_pos, HttpConnection::MAX_BUF_SIZE - m_cur_buf_pos);
        if (read_len < 0)
        {
            int my_errno = errno;
            if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
            {
                LOG_DEBUG("Read len < 0, errno %d, return 0 continue", my_errno);
                return 0;
            }
        }
        
        LOG_DEBUG("HTTP Connection: handle_input read data len %d, socket [%d]", read_len, this->m_sock_handle);
        if (read_len <= 0)
        {
            LOG_INFO("peer close the socket [%d],read_len %d, %s close now", this->m_sock_handle, read_len, strerror(errno));
            return -1;
        }
        
        m_recv_times++;
        m_recv_bytes += read_len;
        
        m_cur_buf_pos += read_len;
        
        m_recv_buffer[m_cur_buf_pos] = '\0';
        m_last_msg_time  = std::time(nullptr);

        return 0;
        //return this->process(m_recv_buffer, m_cur_buf_pos);
    }
    
}

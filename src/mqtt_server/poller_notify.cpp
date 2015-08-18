#include "mqtt_server/poller_notify.hpp"
#include "reactor/poller.hpp"

#include "mqtt_server/subscriber_mgr.hpp"

// for socketpaire
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <sys/eventfd.h>

#include "mqtt_server/xml_config.hpp"

namespace reactor
{
    int CPollerNotifyReader::open(void *)
    {
        return CEventHandler::open();
    }
    
    int CPollerNotifyReader::handle_input(socket_t UNUSED(sock_id))
    {
        // add socket, publish msg etc.
        LOG_TRACE_METHOD(__func__);
        
        int read_len = ::read(this->m_sock_handle, m_recv_buffer + m_cur_buf_pos, CPollerNotifyReader::MAX_BUF_SIZE - m_cur_buf_pos);
        
        LOG_DEBUG("handle_input read data len %d, socket [%d]", read_len, this->m_sock_handle);
        if (read_len <= 0)
        {
            LOG_INFO("peer close the socket [%d],read_len %d, %s close now", this->m_sock_handle, read_len, strerror(errno));
            return -1;
        }
        
        m_recv_times++;
        m_recv_bytes += read_len;
        m_cur_buf_pos = read_len;
        
        {
            // DEAL MSG
            m_cur_buf_pos = 0;
        }
        
        return 0;
    }
    
    int CPollerNotifyWriter::open(void *)
    {
        CEventHandler::open();
        
        if (m_poller_ptr != nullptr)
        {
            m_poller_ptr->regist_notify(this);
        }
        return 0;
    }
    
    CPollerNotifyWriter::~CPollerNotifyWriter()
    {
        if (m_poller_ptr != nullptr)
        {
            m_poller_ptr->unregist_notify();
        }
    }
    
    int CPollerNotify::open()
    {
        int fds[2];
        if (socketpair(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0, fds) == -1)
        {
            LOG_ERROR("socketpair(SOCK_NONBLOCK) failed, %d, %s", errno, strerror(errno));
            return -1;
        }
        
        CPollerNotifyReader *reader = new CPollerNotifyReader(m_poller);
        CPollerNotifyWriter *writer = new CPollerNotifyWriter(m_poller);
        
        reader->set_handle(fds[0]);
        writer->set_handle(fds[1]);
        
        reader->open();
        writer->open();
        
        return 0;
    }
    
    int CPollerNotifyFd::open(void *)
    {
        socket_t eventfd = ::eventfd(0, EFD_CLOEXEC|EFD_NONBLOCK);
        
        this->set_handle(eventfd);
        
        CEventHandler::open();
        
        if (m_poller_ptr != nullptr)
        {
            m_poller_ptr->regist_notify(this);
        }
        return 0;
    }
    
    CPollerNotifyFd::~CPollerNotifyFd()
    {
        if (m_poller_ptr != nullptr)
        {
            m_poller_ptr->unregist_notify();
        }
    }
    
    int CPollerNotifyFd::notify()
    {
        uint64_t value = 1;
        return ::write(this->m_sock_handle, &value, sizeof(value));
    }
    
    int CPollerNotifyFd::handle_input(socket_t )
    {
        uint64_t value = 0;
        read(this->m_sock_handle, &value, sizeof(value));
        
        LOG_DEBUG("Enter CPollerNotifyFd::handle_input()");
        
        // read message from epoll deque and deal
        CMbuf_ptr mbuf;
        while ( m_poller_ptr->pop_front(mbuf) != -1)
        {
            if (mbuf.get() == nullptr)
            {
                LOG_DEBUG("Read msg from poller queue, but no data here");
            }
            
            if (mbuf->msg_type() == MSG_PUBLISH)
            {
                // publish msg
                CMqttPublish publish(mbuf->read_ptr(),mbuf->length());
                if (publish.decode() < 0)
                {
                    LOG_INFO("CPollerNotifyFd::handle_input()::publish decode failed");
                    return -1;
                }
                
                publish.print();
                
                uint32_t start_tm = time(0);
                
                int pub_count = 0;
                if (CONFIG->get_mqtt_bridge() <= 0)
                {
                    LOG_INFO("Publish to topic %s", publish.topic_name().c_str());
                    pub_count = SUB_MGR->publish(publish.topic_name(), mbuf, publish);
                }
                else // bridge
                {
                    LOG_INFO("Publish to all client, bridge mode");
                    pub_count = SUB_MGR->publish_all(mbuf, publish);
                }
                
                uint32_t diff = time(0) - start_tm;
                
                LOG_ERROR("This publish cost %d (s) to %d clients", diff, pub_count);
            }
            else
            {
                LOG_DEBUG("Unknown deque msg type %d", mbuf->msg_type());
            }
        }
        
        return 0;
    }
}


#include "reactor/poller.hpp"
#include "reactor/event_handler.hpp"
#include <stdarg.h>

#include "common/mem_pool.hpp"

extern CMemPool g_mem_pool;

namespace reactor
{
    int my_printf(const char *fmt, ...)
    {
        char buffer[1024];
        va_list argptr;
        int length = 0;
        
        va_start(argptr, fmt);
        length = vsnprintf(buffer,1024,fmt, argptr);
        va_end(argptr);
        
        /*
         buffer[length] = '\n';
         buffer[length + 1] = '\0';
         */
        
        printf("%s\n", buffer);
        
        return (length + 1);
    }
}


namespace reactor
{
    CEventHandler::CEventHandler(CReactor* reactor)
    : m_reactor_ptr(reactor),
    m_recv_bytes(0),
    m_recv_times(0),
    m_send_bytes(0),
    m_send_times(0),
    m_schedule_write_times(0),
    m_cancel_schedule_write_times(0),
    m_max_msg_in_buffer(50),
    m_accept_time(std::time(nullptr))
    
    {
        LOG_TRACE_METHOD(__func__);
    }
    
    CEventHandler::~CEventHandler()
    {
        LOG_TRACE_METHOD(__func__);
    }
    
    void CEventHandler::get_peer_name()
    {
        char client_ip[64];
        char client_addr[64];
        
        struct sockaddr_in client;
        socklen_t client_len = sizeof(client);
        
        getpeername(m_sock_handle, (struct sockaddr *)&client, &client_len);
        inet_ntop(AF_INET, &client.sin_addr, client_ip, sizeof(client_ip));
        
        snprintf(client_addr, 64, "%s:%d", client_ip, ntohs(client.sin_port));
        
        m_str_peer_addr = client_addr;
    }
    
    int CEventHandler::open(void *)
    {
        LOG_TRACE_METHOD(__func__);
        
        if (this->m_reactor_ptr == nullptr)
        {
            LOG_WARN("In CEventHandler::open, poller_ptr is null");
            return -1;
            
        }
        
        this->get_peer_name();
        
        int res = this->m_reactor_ptr->add_event(this, EVENT_READ /*|EVENT_WRITE*/);
        if (res < 0)
        {
            LOG_ERROR("CEventHandler::open(), add event EVENT_READ failed. %s.", strerror(errno));
        }
        
        return res;
        // check res
    }
    
    int CEventHandler::handle_close(socket_t socket)
    {
        LOG_TRACE_METHOD(__func__);
        LOG_DEBUG("In CEventHandler::handle_close() socket %d", socket);
        
        this->close();
        
        delete this;
        
        return 0;
    }
    
    int CEventHandler::handle_output(socket_t socket)
    {
        LOG_TRACE_METHOD(__func__);
        // WIN32 Notes: When the receiver blocked, we started adding to the
        // consumer handler's message Q. At this time, we registered a
        // callback with the reactor to tell us when the TCP layer signalled
        // that we could continue to send messages to the consumer. However,
        // Winsock only sends this notification ONCE, so we have to assume
        // at the application level, that we can continue to send until we
        // get any subsequent blocking signals from the receiver's buffer.
        
        // The list had better not be empty, otherwise there's a bug!
        if (m_send_msg_queue.empty())
        {
            LOG_WARN("In CEventHandler::handle_output, but the send queue is empty");
            this->cancel_schedule_write();
            return 0;
        }
        
        LOG_INFO("Call CEventHandler::handle_output() handle %d", socket);
        
        CMbuf_ptr mbuf = m_send_msg_queue.front();
        m_send_msg_queue.pop_front();
        
        switch(this->nonblk_send(mbuf))
        {
                
            case 0:
                // Partial send.
                // (errno == EWOULDBLOCK);
                // Didn't write everything this time, come back later...
                break;
                
            case -1:
                // We are responsible for releasing Message_Block if
                // failures occur.
                // /* FALLTHROUGH */
                
            default:
                // If we succeed in writing the entire event (or we did not
                // fail due to EWOULDBLOCK) then check if there are more
                // events on the Message_Queue.  If there aren't, tell the
                // ACE_Reactor not to notify us anymore (at least until
                // there are new events queued up).
                break;
        }
        
        if (m_send_msg_queue.empty())
        {
            LOG_DEBUG("CEventHandler::nonblk_send(), queue is empty, cancel wirte event");
            this->cancel_schedule_write();
        }
        
        return 0;
    }
    
    int CEventHandler::schedule_write()
    {
        LOG_TRACE_METHOD(__func__);
        m_schedule_write_times++;
        
        if( (this->m_current_event_mask & EVENT_WRITE) > 0)
        {
            LOG_DEBUG("EV_WRITE(0x%0x) already in event_mask 0x%x",
                      EVENT_WRITE, this->m_current_event_mask);
		          
            return 0;
        }
        
        return this->m_reactor_ptr->mod_event(this, this->m_current_event_mask|EVENT_WRITE);
    }
    
    int CEventHandler::cancel_schedule_write()
    {
        LOG_TRACE_METHOD(__func__);
        m_cancel_schedule_write_times++;
        return this->m_reactor_ptr->mod_event(this, this->m_current_event_mask^EVENT_WRITE);
    }
    
    int CEventHandler::put(CMbuf_ptr &mbuf)
    {
        LOG_TRACE_METHOD(__func__);
        
        LOG_DEBUG("In CEventHandler::put, mbuf.len: %d", mbuf->length());
        
        if (this->m_send_msg_queue.empty())
        {
            LOG_DEBUG("In CEventHandler::put, send msg queue is empty, try call nonblk_send()");
            return this->nonblk_send(mbuf);
        }
        else
        {
            LOG_DEBUG("In CEventHandler::put, send msg queue is not empty, push back msg to queue)");
            uint32_t msg_size = m_send_msg_queue.size();
            if (msg_size >= m_max_msg_in_buffer)
            {
                LOG_INFO("Too many msg in buffer. msgs %d, max_msg %d ", msg_size, m_max_msg_in_buffer);
                return -1;
            }
            
            m_send_msg_queue.push_back(mbuf);
            this->schedule_write();
        }
        
        return 0;
    }
    
    
    int CEventHandler::nonblk_send(CMbuf_ptr &mbuf)
    {
        // Try to send the event.  If we don't send it all (e.g., due to
        // flow control), then re-queue the remainder at the head of the
        // Event_List and ask the ACE_Reactor to inform us (via
        // handle_output()) when it is possible to try again.
        
        int my_errno = 0;
        ssize_t n = this->send (mbuf, my_errno);
        
        if (n == -1)
        {
            // -1 is returned only when things have really gone wrong (i.e.,
            // not when flow control occurs).  Thus, let's try to close down
            // and set up a new reconnection by calling handle_close().
            LOG_ERROR("call send res -1, ready to call handle_close() now");
            this->handle_close ();
            return -1;
        }
        else if ((my_errno == EWOULDBLOCK) || (my_errno == EAGAIN) || (my_errno == EINTR))
        {
            // We didn't manage to send everything, so we need to queue  things up.
            // Queue in *front* of the list to preserve order.
            
            LOG_INFO("Send wouldblock, put msg to queue front");
            this->m_send_msg_queue.push_front(mbuf);
            
            // Tell Reactor to call us back when we can send again.
            this->schedule_write();
            
            return 0;
        }
        else
        {
            return n;
        }
        
        return 0;
    }
    
    int CEventHandler::send(CMbuf_ptr &mbuf, int &my_errno)
    {
        LOG_TRACE_METHOD(__func__);
        
        ssize_t len = mbuf->length ();
        ssize_t n = ::write(this->get_handle(), mbuf->read_ptr(), len);
        my_errno = errno;
        
        // LOG_DEBUG("In CEventHandler::send(), call write res %d, errno %d, %s",n, errno, strerror(errno));
        // this may change errno, so delete this
        if (n <= 0)
        {
            if ((my_errno == EWOULDBLOCK) || (my_errno == EAGAIN) || (my_errno == EINTR))
            {
                LOG_INFO("Write will block, so return now");
                return 0;
            }
            
            return n;
        }
        else
        {
            m_send_times++;
            m_send_bytes += n;
            
            if (n < len)
            {
                // Re-adjust pointer to skip over the part we did send.
                mbuf->read_ptr(n);
                my_errno = EWOULDBLOCK;
            }
            else if (n == len)
            {
                // The whole event is sent, we now decrement the reference count
                // (which deletes itself with it reaches 0).
                errno = 0;
                
                // g_mem_pool.unget(mbuf->copy());
            }
        }
        
        return n;
    }
    
    int CEventHandler::close()
    {
        LOG_TRACE_METHOD(__func__);
        
        this->m_reactor_ptr->del_event(this, 0);      // event_mask not used in epoll
        
        CSockBase::close();
        
        return 0;
    }
    
    int CEventHandler::write_n(uint8_t *buf, uint32_t len)
    {
        uint32_t buf_len = len;
        while (len > 0)
        {
            ssize_t res = ::write(m_sock_handle, buf, len);
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
                        LOG_ERROR("socket(%d) send error, errno %d. %s", m_sock_handle, errno, strerror(errno));
                        return -1;
                    }
                }
            }
            
            len -= res;
        }
        
        return buf_len;
    }
}

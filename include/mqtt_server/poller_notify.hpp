#ifndef _poller_notify__h_
#define _poller_notify__h_

#include "reactor/define.hpp"
#include "reactor/event_handler.hpp"
#include "mqtt_server/mqtt_context.hpp"

class CPoller;

namespace reactor
{
    class CPollerNotifyReader : public CEventHandler
    {
    public:
        enum {MAX_BUF_SIZE = 4096};
        
        CPollerNotifyReader(CReactor *reactor) : CEventHandler(reactor)
        {
        }
        
        virtual int open(void *data = nullptr);
        virtual int handle_input(socket_t sock_id);
        
        // virtual int handle_close(socket_t sock_id = INVALID_SOCKET); // delete this;
    protected:
        uint8_t m_recv_buffer[MAX_BUF_SIZE];
        uint32_t m_cur_buf_pos;
    };
    
    class CPollerNotifyWriter : public CEventHandler
    {
    public:
        CPollerNotifyWriter(CReactor *reactor) : CEventHandler(reactor)
        {
        }
        
        ~CPollerNotifyWriter();                                          // unregist from epoll
        
        virtual int open(void *data=nullptr);                            // regist this to epoller
    };
    
    class CPollerNotify
    {
    public:
        CPollerNotify(CReactor *reactor) : m_reactor(reactor)
        {
        }
        
        int open();
        
    protected:
        CReactor *m_reactor = nullptr;
    };
    
    class CPollerNotifyFd : public CEventHandler
    {
    public:
        CPollerNotifyFd(CReactor *reactor) : CEventHandler(reactor)
        {
        }
        
        virtual int handle_input(socket_t sock_id);
        
        ~CPollerNotifyFd();                                                                                             // ungegist from epoll
        
        virtual int open(void *data = nullptr);
        virtual int notify();
    };
}

#endif



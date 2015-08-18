#ifndef _reactor_poller_epoller_h_
#define _reactor_poller_epoller_h_

#include "reactor/poller.hpp"
#include "common/mbuf.hpp"

namespace reactor
{
    class CPollerEpoll : public CPoller
    {
    public:
        CPollerEpoll();
        ~CPollerEpoll();
        
        int open(int max_fd_size = 1024);
        int close();
        
        virtual bool run(int32_t timeout);
        
        virtual void stop();
        
        virtual int add_event(CEventHandler *tcp_socket, uint32_t event_mask);
        virtual int del_event(CEventHandler *tcp_socket, uint32_t event_mask);
        virtual int mod_event(CEventHandler *tcp_socket, uint32_t event_mask);
        
        virtual void regist_notify(CEventHandler *notify);
        virtual void unregist_notify();
        virtual int notify(CMbuf_ptr &mbuf);
        
        virtual int pop_front(CMbuf_ptr &msg);
        virtual int push_back(CMbuf_ptr &msg);
        
        uint32_t convert_event_mask(uint32_t event_mask);
        
    private:
        struct  epoll_event m_poller_events[MAX_EVENT_SIZE];
    };
}

#endif

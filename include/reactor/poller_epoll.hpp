#ifndef _reactor_poller_epoller_h_
#define _reactor_poller_epoller_h_

#include "reactor/poller.hpp"

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
            
            virtual int add_event(CEventHandler *tcp_socket, uint32_t event_mask);
            virtual int del_event(CEventHandler *tcp_socket, uint32_t event_mask);
            virtual int mod_event(CEventHandler *tcp_socket, uint32_t event_mask);
            
            
            uint32_t convert_event_mask(uint32_t event_mask);
            
        private:
            struct  epoll_event  m_poller_events[MAX_EVENT_SIZE];
        };   
}

#endif

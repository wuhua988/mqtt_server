//
//  tcp_server2.h
//
//
//  Created by davad.di on 7/15/15.
//
//

#ifndef ____tcp_server__
#define ____tcp_server__

#include "mqtt_server/mqtt_connection.hpp"
#include "mqtt_server/acceptor.hpp"

namespace reactor
{
    class TCPServer
    {
    public:
        
        TCPServer():m_server_address(5050), m_acceptor(&m_poller_epoll)
        {
            LOG_TRACE_METHOD(__func__);
        }
        
        int open(CSockAddress &server_addr);
        int loop();
        void stop();
        
    protected:
        
        CSockAddress	m_server_address;
        Acceptor 		m_acceptor;
        CPollerEpoll 	m_poller_epoll;
        bool		m_running_flag = false;
    };
}


#endif /* defined(____tcp_server2__) */

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
#include "reactor/sig_handler.hpp"
#include "reactor/timer_handler.hpp"

namespace reactor
{
    class TCPServer
    {
    public:
        
        TCPServer():m_server_address(5050)
        {
            LOG_TRACE_METHOD(__func__);

	    m_acceptor = new Acceptor(&m_poller_epoll);
	    m_sig_handler = new CSigHandler(&m_poller_epoll);
	    m_timer_handler = new CTimerHandler(&m_poller_epoll);
        }
        
        int open(CSockAddress &server_addr);
        int loop();
        
    protected:
        
        CSockAddress	m_server_address;

        Acceptor 	*m_acceptor = nullptr;	    // manage by handle_close()
        CPollerEpoll 	m_poller_epoll;
        
        CSigHandler      *m_sig_handler = nullptr;  // manage by handle_close()
        CTimerHandler    *m_timer_handler = nullptr; // manage by handle_close()
    };
}


#endif /* defined(____tcp_server2__) */

//
//  tcp_server2.cpp
//
//
//  Created by davad.di on 7/15/15.
//
//

#include <signal.h>
#include "mqtt_server/tcp_server.hpp"

namespace reactor // later -> mqtt_server
{
    int TCPServer::open(CSockAddress &server_addr)
    {
        LOG_TRACE_METHOD(__func__);

        m_server_address = server_addr;

	if (m_persist.restore() == -1)
	{
	    LOG_DEBUG("DB restore failed");
	    return -1;
	}

        if (m_poller_epoll.open() == -1)
        {
            LOG_ERROR("Epoll open faild. %s", strerror(errno));
            return -1;
        }
       
        if ( m_acceptor->open(m_server_address) == -1)
        {
            LOG_ERROR("Open Server at %d failed, %s",
                      m_server_address.get_port(), strerror(errno));
            
            return -1;
        }
        
        LOG_INFO("Open Server at %s:%d succeed.",
                 m_server_address.get_ip().c_str(),m_server_address.get_port() );

	/*
         signal(SIGINT, handle_sigint);
         signal(SIGTERM, handle_sigint);
         
         // ignore sigpipe
         signal(SIGPIPE, SIG_IGN);
         */

        std::set<int> sig_set;
        std::set<int> sig_ign_set;
        
        sig_set.insert(SIGINT);
        sig_set.insert(SIGTERM);
	sig_set.insert(SIGHUP);

        sig_ign_set.insert(SIGPIPE);
        
        m_sig_handler->open(sig_set, sig_ign_set);
	
        m_timer_handler->open(60, 60); // every min to check timeout

        return 0;
    }
    
    int TCPServer::loop()
    {
        LOG_TRACE_METHOD(__func__);
        
        while(m_poller_epoll.run(-1))
        {
        }
       
	m_persist.store(true); // force flush

        LOG_INFO("Ready to exit loop now");
        
        return 0;
    }
    
}

//
//  tcp_server2.cpp
//
//
//  Created by davad.di on 7/15/15.
//
//

#include "mqtt_server/tcp_server.hpp"

// extern int g_run;

namespace reactor // later -> mqtt_server
{
    int TCPServer::open(CSockAddress &server_addr)
    {
        LOG_TRACE_METHOD(__func__);

	m_server_address = server_addr;
        
        if (m_poller_epoll.open() == -1)
        {
            LOG_ERROR("Epoll open faild. %s", strerror(errno));
            return -1;
        }
        
        if ( m_acceptor.open(m_server_address) == -1)
        {
            LOG_ERROR("Open Server at %d failed, %s",
                      m_server_address.get_port(), strerror(errno));
            
            return -1;
        }
        
        LOG_INFO("Open Server at %s:%d succeed.",
                 m_server_address.get_ip().c_str(),m_server_address.get_port() );
        
        m_running_flag = true;
        
        return 0;
    }
    
    int TCPServer::loop(int *stop_flag)
    {
        LOG_TRACE_METHOD(__func__);
        
        while(m_running_flag)
        {
            if ((stop_flag != nullptr) && *stop_flag)
            {
                break;
            }
            
            if (!m_poller_epoll.run(-1)) // -1 wait until event occurs
            {
                LOG_ERROR("Epoller run retuan failed. Exit now");
                break;
            }
        }
        
        LOG_INFO("Ready to exit loop now");
        
        return 0;
    }
    
    void TCPServer::stop()
    {
        LOG_TRACE_METHOD(__func__);
        
        m_running_flag = false;
    }
}

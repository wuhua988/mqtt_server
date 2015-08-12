//
//  http_server.cpp
//
//
//  Created by davad.di on 7/15/15.
//
//
#include <thread>
#include "http_server/http_server.hpp"

namespace http
{
    int HttpServer::open()
    {
	return this->open(m_server_address);
    }

    int HttpServer::open(reactor::CSockAddress &server_addr)
    {
        LOG_TRACE_METHOD(__func__);
        m_server_address = server_addr;

	if (m_poller_epoll.open() == -1)
	{
	    LOG_ERROR("Http Server poller open failed. %d, %s",
					    errno, strerror(errno));
	    return -1;
	}

        if ( m_acceptor->open(m_server_address) == -1)
        {
            LOG_ERROR("Open Server at %d failed, %s",
                      m_server_address.get_port(), strerror(errno));
            
            return -1;
        }
        
        LOG_INFO("Open Server at %s:%d succeed.",
                 m_server_address.get_ip().c_str(),m_server_address.get_port());

	m_thread_handler = std::thread(&HttpServer::svc, this);
        return 0;
    }
    
    int HttpServer::svc()
    {
        LOG_TRACE_METHOD(__func__);
        
        while(m_running_flag)
        {
	    m_poller_epoll.run(1000); // 1000ms
        }
       
        LOG_INFO("Http Server Ready to exit loop now");
        
	return 0;
    } 
}

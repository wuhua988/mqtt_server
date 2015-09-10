//
//  http_server.h
//
//
//  Created by davad.di on 7/15/15.
//
//

#ifndef ____http_server__
#define ____http_server__

#include "http_connection.hpp"
#include "reactor/acceptor.hpp"
#include <thread>

namespace http
{
    class HttpServer
    {
        
    public:
        HttpServer(uint16_t port)
        : m_server_address(port), m_running_flag(true)
        {
            LOG_TRACE_METHOD(__func__);
            m_acceptor = new reactor::CAcceptor<HttpConnection>(&m_reactor);
        }
        
        ~HttpServer()
        {
            m_thread_handler.join();
        }
        
        int open(const reactor::CSockAddress &server_addr);
        int open();
        
        void stop()
        {
            m_running_flag = false;
        }
        
        int svc();
        
    protected:
        reactor::CSockAddress m_server_address;
        reactor::CReactor m_reactor;
        
        reactor::CAcceptor<HttpConnection>            *m_acceptor = nullptr;
        
        bool m_running_flag;
        std::thread m_thread_handler;
    };
}

#endif /* defined(____http_server__) */

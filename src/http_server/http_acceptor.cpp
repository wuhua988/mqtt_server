//
//  http_acceptor.cpp
//
//
//  Created by davad.di on 7/15/15.
//
//

#include "http_server/http_connection.hpp"
#include "http_server/http_acceptor.hpp"

namespace http
{
    int HttpAcceptor::open(const reactor::CSockAddress &address)
    {
        LOG_TRACE_METHOD(__func__);
        
        if (m_sock_acceptor.open(address, 1024) == -1)
        {
            LOG_ERROR("Http Acceptor open failed.");
            return -1;
        }
        
        this->set_handle(m_sock_acceptor.get_handle());
        LOG_INFO("Http Acceptor open succeed. handler %d", m_sock_acceptor.get_handle());
        
        return CEventHandler::open(); // register handler, with EV_READ event
    }
    
    int HttpAcceptor::handle_input(socket_t UNUSED(sock_id))
    {
        LOG_TRACE_METHOD(__func__);
        
        CEventHandler *event_handler = new HttpConnection(this->m_poller_ptr, this->m_notify_poller);
        

        int socket_id = this->m_sock_acceptor.accept(event_handler); 
        LOG_INFO("New http client arrived, sock_id [%d]", socket_id);
        
        event_handler->open();
        
        return 0;
    }
}


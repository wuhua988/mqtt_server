#ifndef _reactor_sock_acceptor_h_
#define _reactor_sock_acceptor_h_

//#ifdef HAVE_EPOLL

#include "reactor/sock_address.hpp"
#include "reactor/sock_base.hpp"
#include "reactor/event_handler.hpp"

namespace reactor
{
    class CSockAcceptor : public CSockBase
    {
    public:
        CSockAcceptor();
        ~CSockAcceptor();
        
        virtual int open(const CSockAddress &addr, int max_listen = 1024);
        virtual int close();
        
        socket_t accept(CEventHandler *tcp_socket);
        
    protected:
        CSockAddress m_sock_address;
    };
}

// #endif // HAVE_EPOLL

#endif


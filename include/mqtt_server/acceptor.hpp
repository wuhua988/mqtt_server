//
//  acceptor.h
//
//
//  Created by davad.di on 7/15/15.
//
//

#ifndef ____acceptor__
#define ____acceptor__

#include "reactor/define.hpp"
#include "reactor/poller_epoll.hpp"
#include "reactor/sock_acceptor.hpp"
#include "reactor/event_handler.hpp"

namespace reactor
{
    class Acceptor : public CEventHandler
    {
    public:
        Acceptor(CPoller *poller) : CEventHandler(poller)
        {
            LOG_TRACE_METHOD(__func__);
        }
        
        int open(const CSockAddress &address);
        
        virtual int handle_input(socket_t sock_id);
        
    protected:
        CSockAcceptor m_sock_acceptor;
        
    private:
        ~Acceptor()
        {
            LOG_TRACE_METHOD(__func__);
        }
    };
}

#endif /* defined(____acceptor__) */

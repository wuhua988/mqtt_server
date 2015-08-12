//
//  acceptor.h
//
//
//  Created by davad.di on 7/15/15.
//
//

#ifndef ____http_acceptor__
#define ____http_acceptor__

#include "reactor/define.hpp"
#include "reactor/poller_epoll.hpp"
#include "reactor/sock_acceptor.hpp"
#include "reactor/event_handler.hpp"

namespace http
{
    class HttpAcceptor : public reactor::CEventHandler
    {
    public:
        HttpAcceptor(reactor::CPoller *poller, reactor::CPoller *notify_poller)
	    : reactor::CEventHandler(poller), m_notify_poller(notify_poller)
        {
            LOG_TRACE_METHOD(__func__);
        }
        
        int open(const reactor::CSockAddress &address);
        virtual int handle_input(socket_t sock_id);

    protected:
	reactor::CSockAcceptor  m_sock_acceptor;

	reactor::CPoller	*m_notify_poller = nullptr;
    private:
	~HttpAcceptor()
	{
	    LOG_TRACE_METHOD(__func__);
	}
    };
}

#endif /* defined(____http_acceptor__) */

//
//  acceptor.h
//
//
//  Created by davad.di on 7/15/15.
//
//

#ifndef _____acceptor__
#define _____acceptor__

#include "reactor/define.hpp"
#include "reactor/poller_epoll.hpp"
#include "reactor/sock_acceptor.hpp"
#include "reactor/event_handler.hpp"

namespace reactor
{
    template <class T>
        class CAcceptor : public reactor::CEventHandler
    {
        public:
            CAcceptor(reactor::CReactor *reactor)
                : reactor::CEventHandler(reactor)
            {
                LOG_TRACE_METHOD(__func__);
            }

            int open(const reactor::CSockAddress &address)
            {
                LOG_TRACE_METHOD(__func__);

                if (m_sock_acceptor.open(address, 1024) == -1)
                {
                    LOG_ERROR("CAcceptor open failed.");
                    return -1;
                }

                this->set_handle(m_sock_acceptor.get_handle());
                LOG_INFO("CAcceptor open succeed. handler %d", m_sock_acceptor.get_handle());

                return CEventHandler::open(); // register handler, with EV_READ event
            }

            virtual int handle_input(socket_t UNUSED(sock_id))
            {
                LOG_TRACE_METHOD(__func__);

                CEventHandler *event_handler = new T(this->m_reactor_ptr);


                int socket_id = this->m_sock_acceptor.accept(event_handler);
                LOG_INFO("New  client arrived, sock_id [%d]", socket_id);

                event_handler->open();

                return 0;
            }


        protected:
            reactor::CSockAcceptor m_sock_acceptor;

        private:
            ~CAcceptor()
            {
                LOG_TRACE_METHOD(__func__);
            }
    };
}

#endif /* defined(____http_acceptor__) */

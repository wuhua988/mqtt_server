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
#include "mqtt_server/timer_handler.hpp"
#include "mqtt_server/persist_msg_pack.hpp"

#include "mqtt_server/poller_notify.hpp"

namespace reactor
{
    class TCPServer
    {
        
    public:
        TCPServer(std::string &file_name, CReactor *reactor)
        : m_server_address(5050), m_reactor(reactor),  m_persist(file_name)
        {
            LOG_TRACE_METHOD(__func__);
            
            m_acceptor = new Acceptor(reactor);
            m_sig_handler = new CSigHandler(reactor);
            m_timer_handler = new CTimerHandler(reactor, &m_persist);
            
            m_notify_fd  = new CPollerNotifyFd(reactor);
        }
        
        int open(CSockAddress &server_addr);
        int loop();
        
    protected:
        
        CSockAddress m_server_address;
        
        Acceptor        *m_acceptor = nullptr;                      // manage by handle_close()
        CReactor        *m_reactor = nullptr;
        
        CPollerNotifyFd  *m_notify_fd = nullptr;
        CSigHandler      *m_sig_handler = nullptr;                  // manage by handle_close()
        CTimerHandler    *m_timer_handler = nullptr;                 // manage by handle_close()
        CPersistMsgPack m_persist;
    };
}


#endif /* defined(____tcp_server2__) */

//
//  mqtt_connection.h
//  
//
//  Created by davad.di on 7/15/15.
//
//

#ifndef ____mqtt_connection__
#define ____mqtt_connection__

#include "reactor/define.hpp"
#include "reactor/poller_epoll.hpp"
#include "reactor/event_handler.hpp"

namespace reactor
{
    class CMqttConnection : public CEventHandler
    {
    public:
        CMqttConnection(CPoller *poller) : CEventHandler(poller)
        {
            LOG_TRACE_METHOD(__func__);
        }
        
        ~CMqttConnection()
        {
            LOG_TRACE_METHOD(__func__);
        }
        
        int process_http(CMbuf_ptr &mbuf);
        
        int process_echo(CMbuf_ptr &mbuf);
        int process_echo();
        int process_mqtt(CMbuf_ptr &mbuf);
        
        
        virtual int handle_input(socket_t sock_id);
        virtual int handle_close(socket_t sock_id);
    };
}

#endif /* defined(____mqtt_connection__) */

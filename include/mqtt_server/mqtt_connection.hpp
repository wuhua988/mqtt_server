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
#include "reactor/event_handler.hpp"
#include "mqtt_server/mqtt_context.hpp"

class CPoller;

namespace reactor
{
    class CMqttConnection : public CEventHandler
    {
        enum {MAX_BUF_SIZE = 4096};
        
    public:
        CMqttConnection(CReactor *reactor) : CEventHandler(reactor), m_cur_buf_pos(0)
        {
            LOG_TRACE_METHOD(__func__);
            m_accept_time = std::time(nullptr);
        }
        
        ~CMqttConnection()
        {
            LOG_TRACE_METHOD(__func__);
        }
        
        CReactor * reactor()
        {
            return this->m_reactor_ptr;
        }
        
        CMqttClientContext_ptr & client_context()
        {
            if ( m_client_context.get() == nullptr )
            {
                m_client_context = make_shared<CTMqttClientContext>(this);
            }
            
            return m_client_context;
        }
        
        void client_context(CMqttClientContext_ptr & client_context)
        {
            m_client_context = client_context;
        }
        
        virtual int handle_input(socket_t sock_id);
        virtual int handle_close(socket_t sock_id = INVALID_SOCKET);
        
        // mqtt logic
        int process_mqtt(uint8_t *buf, uint32_t len);
        int process(uint8_t *buf, uint32_t len, CMqttConnection *mqtt_connection);
        
        int handle_connect_msg(uint8_t *buf, uint32_t len, CMqttConnection *mqtt_connection);
        int handle_publish_msg(uint8_t *buf, uint32_t len, CMqttConnection *mqtt_connection);
        int handle_puback_msg(uint8_t *buf, uint32_t len, CMqttConnection *mqtt_connection);
        int handle_subscribe_msg(uint8_t *buf, uint32_t len, CMqttConnection *mqtt_connection);
        int handle_unsubscribe_msg(uint8_t *buf, uint32_t len, CMqttConnection *mqtt_connection);
        int handle_pingreq_msg(uint8_t *buf, uint32_t len, CMqttConnection *mqtt_connection);
        int handle_disconnect_msg(uint8_t *buf, uint32_t len, CMqttConnection *mqtt_connection);
        
        uint32_t last_msg_time()
        {
            return m_last_msg_time;
        }
        
    protected:
        CMqttClientContext_ptr m_client_context;
        
        uint8_t m_recv_buffer[MAX_BUF_SIZE];
        uint32_t m_cur_buf_pos;
        
        uint32_t m_accept_time;
        uint32_t m_last_msg_time;
    };
}

#endif /* defined(____mqtt_connection__) */

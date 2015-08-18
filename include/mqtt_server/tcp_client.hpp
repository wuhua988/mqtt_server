#ifndef _tcp_client_h__
#define _tcp_client_h__

#include "reactor/event_handler.hpp"
#include <thread>
#include "reactor/sock_address.hpp"


namespace reactor
{
    
    class CPoller;
    
    class TCPClient
    {
    public:
        enum {MAX_BUF_SIZE = 4096};
        
        TCPClient(CPoller *poller, std::string topic_name, std::string client_id)
        : m_running_flag(true),
        m_poller(poller),
        m_str_client_id(client_id),
        m_str_topic_name(topic_name)
        {
        }
        
        ~TCPClient()
        {
            m_thread_handler.join();
        }
        
        virtual int open(void *data = nullptr);                // start a thead to deal
        int connect();
        
        void svc();
        
        void stop()
        {
            m_running_flag = false;
        }
        
        int     write_n(uint8_t *buf, uint32_t len);
        int     read(uint8_t *buf, uint32_t len, uint32_t tm);          // tm : ms
        
        int     process(uint8_t *buf, uint32_t len);
        int     process_mqtt(uint8_t *buf, uint32_t len);          // whole mqtt_pkt
        // parent handle_close
        
        // mqtt msg
        int send_pingreq_msg();
        int send_connect_msg();
        int send_disconnect_msg();
        
        int send_subscribe_msg(uint16_t msg_id);
        int send_pub_ack_msg(uint16_t msg_id);
        
    protected:
        uint8_t m_recv_buffer[MAX_BUF_SIZE];
        uint32_t m_cur_buf_pos;
        
        socket_t m_client_socket;
        bool m_running_flag;
        CSockAddress m_server_addr;
        std::thread m_thread_handler;
        
        CPoller     *m_poller;
        
        std::string m_str_client_id;
        std::string m_str_topic_name;
        uint8_t m_topic_qos;
    };
}

#endif


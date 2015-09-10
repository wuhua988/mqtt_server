#ifndef _tcp_client_h__
#define _tcp_client_h__

#include "reactor/event_handler.hpp"
#include <thread>
#include "reactor/sock_address.hpp"


namespace reactor
{
    
    class CReactor;
    
    class CTCPClient : public CEventHandler
    {
    public:
        enum {MAX_BUF_SIZE = 4096};
        
        CTCPClient(CSockAddress server_addr, CReactor *reactor) 
            : CEventHandler(reactor), m_server_addr(server_addr) 
        {
        }
        
        void hello();

        CTCPClient(const char *srv_ip, uint16_t srv_port, CReactor *reactor) : CEventHandler(reactor)
        {
            CSockAddress srv_addr(srv_ip, srv_port);
            m_server_addr = srv_addr;
        }
        
        CTCPClient(CReactor *reactor) : CEventHandler(reactor)
        {
        }

        ~CTCPClient();
        
        virtual int open(void *data = nullptr);
        
        int connect();
        int connect(const char *ip, uint16_t port);
     
        virtual int handle_input(socket_t socket);
        virtual int handle_output(socket_t socket);
        virtual int handle_timeout(uint32_t time, void *data = nullptr);
        virtual int handle_close(socket_t socket = INVALID_SOCKET);

    protected:
        uint8_t m_recv_buffer[MAX_BUF_SIZE];
        uint32_t m_cur_buf_pos;
        
        //socket_t m_client_socket = INVALID_SOCKET;
        CSockAddress m_server_addr;

        CONNECT_STATUS  m_client_status = CONNECT_STATUS::CLIENT_UNCONNECTED;

        int m_timer_id = 0; // connect timer
        int m_timeout_value = 8; // reconnet interval

        int m_timer_id_data = 0; // 60s
        int m_timeout_value_data = 60; 
    };
}

#endif


#include "reactor/tcp_client.hpp"
#include "reactor/reactor.hpp"

namespace reactor
{
    CTCPClient::~CTCPClient()
    {
        if (m_timer_id != 0)
        {
            this->reactor()->unregist_timer(m_timer_id);
        }
    }

    int CTCPClient::open(void *)
    {
        // regist EV_READ ?
        return 0;
    }

    void CTCPClient::hello()
    {
        // send data
        this->write_n((uint8_t *)"Hello", 5);
    }

    int CTCPClient::connect(const char *ip, uint16_t port)
    {
        CSockAddress srv_addr(ip, port);
        m_server_addr = srv_addr;

        return this->connect();
    }

    int CTCPClient::connect()
    {
        LOG_DEBUG("Enter CTCPClient connect()");

        this->m_sock_handle = socket(AF_INET,SOCK_STREAM,0);
        LOG_DEBUG("TCP Client_socket %d", this->m_sock_handle);

        if( this->m_sock_handle < 0)
        {
            LOG_ERROR("Create socket failed, errno %d, %s", errno, strerror(errno));
            return -1;
        }

        this->set_non_block();

        struct sockaddr_in server_addr = m_server_addr.sock_addr();
        socklen_t server_addr_length = sizeof(server_addr);

        // uint32_t timeout = 1;
        int res = -1;

        m_client_status = CONNECT_STATUS::CLIENT_CONNECTING;

        res = ::connect(this->m_sock_handle,(struct sockaddr*)&server_addr, server_addr_length);

        int my_errno = errno;
        if (res < 0)
        {
            LOG_DEBUG("Connect res %d, res %d, %s", res, my_errno, strerror(my_errno));

            if (my_errno == EINPROGRESS || my_errno == EWOULDBLOCK)
            {
                LOG_DEBUG("Connect to server failed, EINPROGRESS");
                // regist to reactor
                // create NOBLOCK_CONNNECTOR object
                // handle_input or handle_close failed. handle_output succeed.
                this->reactor()->add_event(this, EVENT_READ|EVENT_WRITE|EVENT_ERROR);
                LOG_DEBUG("Add client event to reactor");
            }

            if (m_timer_id == 0)
            {
                m_timer_id = this->reactor()->regist_timer(this, m_timeout_value); // only one time
                LOG_DEBUG("Client regist timer to reactor id %d, timeout %d", m_timer_id, m_timeout_value);
            }
        }
        else
        {
            m_client_status =  CONNECT_STATUS::CLIENT_CONNECTED;

            LOG_DEBUG("connect(): TCP Client connect to %s:%d succeed.",
                    m_server_addr.get_ip().c_str(), m_server_addr.get_port());

            this->hello();

            // send connect msg
            m_cur_buf_pos = 0;
        }

        return 0;
    }


    int CTCPClient::handle_input(socket_t)
    {
        LOG_DEBUG("Enter into handle_input");
        // recv data
        char buf[1204];
        int res = ::read(this->m_sock_handle, buf, 1024);
        LOG_DEBUG("Recv data len %d", res);

        if (res <= 0)
        {
            return -1;
        }

        return 0;
    }

    int CTCPClient::handle_timeout(uint32_t time, void *)
    {
        // reconnect again

        LOG_DEBUG("In fun handle_timeout()  tm %d", time);
        if (m_client_status != CONNECT_STATUS::CLIENT_CONNECTED)
        {
            LOG_DEBUG("Start Reconnect now");
            this->connect();
        }
        else
        {
            LOG_DEBUG("Send heartbeat msg");
        }

        return 0;
    }

    int CTCPClient::handle_output(socket_t socket)
    {
        LOG_DEBUG("Enter into handle_output");

        if (m_client_status == CONNECT_STATUS::CLIENT_CONNECTING)
        {
            // connect succeed.
            m_client_status = CONNECT_STATUS::CLIENT_CONNECTED;

            this->reactor()->mod_event(this, EVENT_READ);

            if (m_timer_id != 0)
            {
                // mode timer to data timer
            }

            LOG_DEBUG("handle_output(): TCP Client connect to %s:%d succeed.",
                    m_server_addr.get_ip().c_str(), m_server_addr.get_port());

            this->hello();
        }
        else if (m_client_status == CONNECT_STATUS::CLIENT_CONNECTED)
        {
            CEventHandler::handle_output(socket);
        }

        return 0;
    }

    int CTCPClient::handle_close(socket_t)
    {
        this->close();

        LOG_DEBUG("Enter into handle_close(), Start reconnect now");
        //this->connect(); ->  connect() on handle_timeout
        m_client_status = CONNECT_STATUS::CLIENT_UNCONNECTED;
        return 0;
    }
}


#ifndef _reactor_sock_address_h_
#define _reactor_sock_address_h_

#include "reactor/define.hpp"  // also include c++ header

namespace reactor
{
    class CSockAddress
    {
        
    public:
        CSockAddress()
        {
            LOG_TRACE_METHOD(__func__);
        }
        
        CSockAddress(uint16_t port)
        {
            LOG_TRACE_METHOD(__func__);
            
            if (!port)
            {
                return;
            }
            
            this->m_str_ip = "0.0.0.0";
            this->m_port = port;
            
            this->init();
        }
        
        CSockAddress(std::string ip, uint16_t port)
        {
            LOG_TRACE_METHOD(__func__);
            
            this->m_str_ip = ip;
            this->m_port = port;
            
            this->init();
        }
        
        CSockAddress(const struct sockaddr_in& address)
        {
            LOG_TRACE_METHOD(__func__);
            
            this->m_str_ip = inet_ntoa(address.sin_addr);
            this->m_port = address.sin_port;
            
            this->init();
        }
        
        std::string get_ip()
        {
            LOG_TRACE_METHOD(__func__);
            return this->m_str_ip;
        }
        
        uint16_t get_port()
        {
            LOG_TRACE_METHOD(__func__);
            return this->m_port;
        }
        
        struct sockaddr_in & sock_addr()
        {
            LOG_TRACE_METHOD(__func__);
            return m_sock_address;
        }
    private:
        
        void init()
        {
            LOG_TRACE_METHOD(__func__);
            
            this->m_sock_address.sin_family = AF_INET;
            this->m_sock_address.sin_addr.s_addr = inet_addr(this->m_str_ip.c_str());
            this->m_sock_address.sin_port = htons(this->m_port);
        }
        
    private:
        sockaddr_in m_sock_address;
        std::string m_str_ip;
        uint16_t m_port;
    };
}

#endif

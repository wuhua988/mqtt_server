//
//  http_connection.h
//
//
//  Created by davad.di on 7/15/15.
//
//

#ifndef ____http_connection__
#define ____http_connection__

#include "reactor/define.hpp"
#include "reactor/event_handler.hpp"

//class reactor::CPoller;

namespace http
{
    class HTTPRequest
    {
    public:
        HTTPRequest()
        {
        }
        
        void print()
        {
            LOG_DEBUG("Methord %s, path %s, params %s", method.c_str(), path.c_str(), params.c_str());
            
            
            LOG_DEBUG("Headers:");
            int i = 0;
            for (auto it = headers.begin(); it != headers.end(); it++)
            {
                LOG_DEBUG("\t[%d] [%s] -> [%s]", ++i, it->first.c_str(), it->second.c_str());
            }
            
            
            i = 0;
            LOG_DEBUG("Querys:");
            for (auto it = query.begin(); it != query.end(); it++)
            {
                LOG_DEBUG("\t[%d] [%s] -> [%s]", ++i, it->first.c_str(), it->second.c_str());
            }
            
        }
        
        int query_param(std::string name, std::string &value)
        {
            auto it = query.find(name);
            if (it != query.end())
            {
                value = it->second;
                return 0;
            }
            
            return -1;
        }
        
        std::string method;
        std::string path;
        std::string params;
        
        std::map<std::string, std::string> headers;
        std::map<std::string, std::string> query;
        std::map<std::string, std::string> cookies;
    };
    
    class HTTPResponse
    {
    public:
        HTTPResponse()
        {
            code = 200;
            phrase = "OK";
            type = "text/html";
            body << "";
            
            /* set current date and time for "Date: " header */
            char buffer[100];
            std::time_t now = std::time(NULL);
            std::strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S %Z", std::gmtime(&now));
            
            date = buffer;
        }
        
        int code;
        std::string phrase;
        std::string type;
        std::string date;
        std::stringstream body;
        
        void append(std::string str)
        {
            body << str;
        };
    };
    
    
    class HttpConnection : public reactor::CEventHandler
    {
        enum {MAX_BUF_SIZE = 4096};
        
    public:
        HttpConnection(reactor::CPoller *poller, reactor::CPoller *notify_poller)
        : reactor::CEventHandler(poller), m_cur_buf_pos(0), m_notify_poller(notify_poller)
        {
            LOG_TRACE_METHOD(__func__);
        }
        
        ~HttpConnection()
        {
            LOG_TRACE_METHOD(__func__);
        }
        
        reactor::CPoller * poller()
        {
            return this->m_poller_ptr;
        }
        
        virtual int handle_input(socket_t sock_id);
        // virtual int handle_close(socket_t sock_id = INVALID_SOCKET);
        
        // http logic
        int process(uint8_t *buf, uint32_t len);
        int parse_headers(char* headers, HTTPRequest* req, HTTPResponse* res);
        int http_send(const char *data, uint32_t len);
        
        int notify_mqtt_publish(HTTPRequest &req, std::string &error);
        
        time_t last_msg_time()
        {
            return m_last_msg_time;
        }
        
    protected:
        uint8_t m_recv_buffer[MAX_BUF_SIZE];
        uint32_t m_cur_buf_pos;
        time_t m_last_msg_time;
        
        reactor::CPoller *m_notify_poller = nullptr;
    };
}

#endif /* defined(___http_connection__) */

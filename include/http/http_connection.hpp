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

#include "http/HttpParserWrapper.h"

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
        HttpConnection(reactor::CReactor *reactor)
        : reactor::CEventHandler(reactor), m_cur_buf_pos(0)
        {
            LOG_TRACE_METHOD(__func__);
        }
        
        ~HttpConnection()
        {
            //this->m_reactor_ptr->unregist_timer(m_timer_id);
            LOG_TRACE_METHOD(__func__);
        }

        virtual int open(void *)
        {
            return reactor::CEventHandler::open();
            // m_timer_id = this->m_reactor_ptr->regist_timer(this, 5, 0); // 5s
            //LOG_DEBUG("Regist timer id %d", m_timer_id);
        }

        virtual int handle_timeout(uint32_t tm, void *)
        {
            LOG_DEBUG("Handle_timeout %d", tm);
            return 0;
        }

        virtual int handle_input(socket_t sock_id);

        int http_send(HTTPResponse &res);

        virtual int process(CHttpParserWrapper &http_parse, HTTPResponse &http_response) = 0;
        // virtual int handle_close(socket_t sock_id = INVALID_SOCKET);
        
    protected:
        uint8_t m_recv_buffer[MAX_BUF_SIZE];
        uint32_t m_cur_buf_pos;
        std::time_t m_last_msg_time;
        
        int m_timer_id;

        HTTPResponse       m_http_response;
        CHttpParserWrapper m_http_parser;
    };
}

#endif /* defined(___http_connection__) */

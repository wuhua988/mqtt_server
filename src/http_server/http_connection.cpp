#include "http_server/http_connection.hpp"

#include "common/str_tools.hpp"
#include "common/mbuf.hpp"

#include "mqttc++/mqtt_msg.hpp"
#include "common/msg_mem_store.hpp"

#include "reactor/poller.hpp"

namespace http
{
    int HttpConnection::handle_input(socket_t)
    {
        LOG_TRACE_METHOD(__func__);
        
        int read_len = ::read(this->m_sock_handle, m_recv_buffer + m_cur_buf_pos, HttpConnection::MAX_BUF_SIZE - m_cur_buf_pos);
        if (read_len < 0)
        {
            int my_errno = errno;
            if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
            {
                LOG_DEBUG("Read len < 0, errno %d, return 0 continue", my_errno);
                return 0;
            }
        }
        
        LOG_DEBUG("HTTP Connection: handle_input read data len %d, socket [%d]", read_len, this->m_sock_handle);
        if (read_len <= 0)
        {
            LOG_INFO("peer close the socket [%d],read_len %d, %s close now", this->m_sock_handle, read_len, strerror(errno));
            return -1;
        }
        
        m_recv_times++;
        m_recv_bytes += read_len;
        
        m_cur_buf_pos += read_len;
        
        m_recv_buffer[m_cur_buf_pos] = '\0';
        m_last_msg_time  = std::time(nullptr);
        return this->process(m_recv_buffer, m_cur_buf_pos);
    }
    
#define json_error "{\n" \
"\"error\":\"%s\",\n" \
"\"error_code\":%s,\n" \
"\"request\":\"%s\"\n" \
"}\n"
    
    int HttpConnection::process(uint8_t *buf, uint32_t UNUSED(len))
    {
        LOG_DEBUG("HTTP Content \n%s", (char *)buf);
        
        HTTPRequest req;
        HTTPResponse response;
        
        int res = this->parse_headers((char *)buf, &req, &response);
        LOG_DEBUG("parse_headers res %d, req.path [%s]", res, req.path.c_str());
        
        if (req.path.compare("/favicon.ico") == 0)
        {
            LOG_DEBUG("Find get favicon request, just return now");
            return -1; // just ignore
        }
        
        std::string error;
        
        response.type = "text/json";
        char str_buf[1024];
        if ( (res == -1) || (req.path.compare("/publish") != 0))
        {
            LOG_DEBUG("Send not found 404 to client");
            response.code = 404;
            response.phrase = "Not Found";
            
            snprintf(str_buf, 1024, json_error, "not found", "400", req.path.c_str());
            std::string str(str_buf);
            response.append(str);
            /*
             response.append("<html>\n"\
             "<head><title>404 Not Found</title></head>\n" \
             "<body bgcolor=\"white\">\n" \
             "<center><h1>404 Not Found</h1></center>\n" \
             "<hr><center>http_server  / 1.0.0</center>\n" \
             "</body>\n" \
             "</html>\n");
             */
        }
        else
        {
            if (this->notify_mqtt_publish(req, error) == -1)
            {
                response.code = 400;
                snprintf(str_buf, 1024, json_error, error.c_str(), "400", req.path.c_str());
                std::string str(str_buf);
                response.append(str);
            }
            else
            {
                response.code = 200;
                response.append("{\"msg\":\"succeed\"}");
            }
            
            /*
             response.append("<html>\n"\
             "<head><title>Succeed</title></head\n" \
             "<body bgcolor=\"white\">\n" \
             "<center><h1>");
             
             response.append(error);
             
             response.append("</h1></center>\n</body>\n<html>\n");
             */
        }
        
        this->http_send(response.body.str().c_str(), strlen(response.body.str().c_str()));
        
        return 0; //wait for close socket
    }
    
    int HttpConnection::notify_mqtt_publish(HTTPRequest &req, std::string &error)
    {
        // make mqtt publish msg and notify to mqtt server
        std::string str_client_id;
        std::string str_topic_name;
        std::string str_qos;
        std::string str_retain_flag;
        std::string str_msg;
        
        uint8_t retain = 0;
        uint8_t qos = 1;
        
        error.clear();
        
        // must be
        if (req.query_param("client", str_client_id) == -1)
        {
            // error description
            error += "client param missed. ";
        }
        
        // msut be
        if (req.query_param("topic", str_topic_name) == -1)
        {
            // error descripiton
            error += "topic param missed. ";
        }
        
        // must be
        if (req.query_param("msg",str_msg) == -1)
        {
            // error description
            error += "msg param missed";
        }
        
        // optional
        if (req.query_param("qos", str_qos) != -1)
        {
            qos = atoi(str_qos.c_str());
            if (qos > 1)
            {
                qos = 1;
            }
        }
        
        // opational
        if (req.query_param("retain", str_retain_flag) == -1)
        {
            retain = atoi(str_retain_flag.c_str());
            if (retain > 1)
            {
                retain = 1;
            }
        }
        
        CMqttFixedHeader fixed_header;
        fixed_header.msg_type(MqttType::PUBLISH);
        fixed_header.retain_flag(retain);
        fixed_header.qos(qos);
        
        uint32_t buf_len = str_msg.length() + str_topic_name.length() + 8;
        CMbuf_ptr mbuf = make_shared<CMbuf>(buf_len);
        mbuf->msg_type(MSG_PUBLISH);
        
        uint64_t publish_msg_id = MSG_MEM_STORE->next_msg_id();   // change msg_id to msg_id
        mbuf->msg_id(publish_msg_id);
        mbuf->regist_mem_store(MSG_MEM_STORE);
        
        CMqttPublish mqtt_publish(mbuf->read_ptr(), buf_len, fixed_header);
        
        
        mqtt_publish.msg_id((uint16_t)(publish_msg_id&0xFFFF));
        mqtt_publish.topic_name(str_topic_name);
        
        /*str msg may decode for some reason*/
        
        std::vector<uint8_t> payload;
        for (auto it = str_msg.begin(); it != str_msg.end(); it++)
        {
            payload.push_back(*it);
        }
        
        mqtt_publish.payload(payload);
        
        int pkt_len = mqtt_publish.encode();
        if (pkt_len == -1)
        {
            error += "publish msg encode failed";
        }
        else
        {
            mbuf->write_ptr(pkt_len);
            if (m_notify_poller != nullptr)
            {
                LOG_DEBUG("Publish msg lenght %d", mbuf->length());
                ::hex_dump(mbuf->read_ptr(), mbuf->length());
                m_notify_poller->notify(mbuf);
            }
        }
        
        if (!error.empty())
        {
            return -1;
        }
        
        return 0;
    }
    
    int HttpConnection::parse_headers(char* headers, HTTPRequest* req, HTTPResponse* UNUSED(res))
    {
        int i = 0;
        char * pch;
        char * context;
        for (pch = strtok_safe(headers, "\n", &context); pch; pch = strtok_safe(NULL, "\n", &context))
        {
            std::string line(pch);
            str_tools::trim(line);
            
            if (line.empty()) // header \r\n\r\n body
            {
                continue;
            }
            
            LOG_DEBUG("Line [%s], size %d", line.c_str(), (int)line.length());
            
            if (i++ == 0)
            {
                std::vector<std::string> R;
                str_tools::split(line, " ", 3, &R);
                
                if (R.size() != 3) {
                    /* throw error */
                    LOG_DEBUG("stop at R.size() != 3");
                    return -1;
                }
                
                req->method = R[0];
                req->path = R[1];
                
                size_t pos = req->path.find('?');
                
                /* We have GET params here */
                if (pos != std::string::npos)
                {
                    std::vector<std::string> Q1;
                    str_tools::split(req->path.substr(pos + 1), "&", -1, &Q1);
                    
                    for (std::vector<std::string>::size_type q = 0; q < Q1.size(); q++)
                    {
                        std::vector<std::string> Q2;
                        str_tools::split(Q1[q], "=", -1, &Q2);
                        
                        if (Q2.size() == 2)
                        {
                            req->query[Q2[0]] = Q2[1];
                        }
                    }
                    
                    req->path = req->path.substr(0, pos);
                }
                else
                {
                    if (req->method.compare("POST") == 0)
                    {
                        str_tools::remove_space(req->path);
                    }
                }
            }
            else
            {
                std::vector<std::string> R;
                str_tools::split(line, ": ", 2, &R);
                
                if (R.size() == 2) {
                    req->headers[R[0]] = R[1];
                    
                    /* Yeah, cookies! */
                    /*
                     if (R[0] == "Cookie") {
                     std::vector<std::string> C1;
                     str_tools::split(R[1], "; ", -1, &C1);
                     
                     for (std::vector<std::string>::size_type c = 0; c < C1.size(); c++)
                     {
                     std::vector<std::string> C2;
                     
                     str_tools::split(C1[c], "=", 2, &C2);
                     
                     req->cookies[C2[0]] = C2[1];
                     }
                     }
                     */
                }
                else
                {
                    /* for POST method */
                    if (req->method.compare("POST") == 0)
                    {
                        LOG_DEBUG("Deal POST method");
                        R.clear();
                        
                        str_tools::split(line, "&", 3, &R);
                        for (std::vector<std::string>::size_type j = 0; j < R.size(); ++j)
                        {
                            std::vector<std::string> data;
                            str_tools::split(R[j], "=", 2, &data);
                            req->query[data[0]] = data[1];
                        }
                    }
                }
            }
        }
        
        return 0;
    }                             // end of parse header
    
    int HttpConnection::http_send(const char* data, uint32_t length)
    {
        HTTPResponse res;
        char header_buffer[1024];
        std::string body;
        body.append(data, length);
        
        /* build http response */
        snprintf(header_buffer,
                 sizeof(header_buffer),
                 "HTTP/1.0 %d %s\r\n" \
                 "Server: %s %s\r\n" \
                 "Date: %s\r\n" \
                 "Content-Type: %s\r\n" \
                 "Content-Length: %d\r\n",
                 res.code, res.phrase.c_str(),
                 "HttpServer", "1.0",
                 res.date.c_str(),
                 res.type.c_str(),
                 length
                 );
        
        /* append extra crlf to indicate start of body */
        strcat_safe(header_buffer, "\r\n");
        
        std::string response_data = "";
        response_data.append(header_buffer, strlen(header_buffer));
        response_data.append(body);
        
        return this->write_n((uint8_t *)response_data.c_str(), response_data.size());
    }
}

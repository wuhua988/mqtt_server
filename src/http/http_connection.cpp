
#include "http/http_connection.hpp"
#include "common/str_tools.hpp"
#include "common/mbuf.hpp"
#include "common/msg_mem_store.hpp"

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
        if (read_len < 0)
        {
            LOG_INFO("peer close the socket [%d],read_len %d, %s close now", this->m_sock_handle, read_len, strerror(errno));
            return -1;
        }

        m_recv_times++;
        m_recv_bytes += read_len;
        m_cur_buf_pos += read_len;

        m_recv_buffer[m_cur_buf_pos] = '\0';
        m_last_msg_time  = std::time(nullptr);

        m_http_parser.ParseHttpContent((const char*)m_recv_buffer, m_cur_buf_pos);

        bool read_all = m_http_parser.IsReadAll();
        
        if (read_len == 0 && !read_all)
        {
            LOG_DEBUG("Client close the socket, but we don't have whole pkt to parser, something wrong!");
            return -1; // client close socket, but we don't have whole pkt.
        }

        if (read_all) // we have got whole pkt 
        {
            this->process(m_http_parser, m_http_response);
            this->http_send(m_http_response);

            return -1; // close socket
        }

        // IsReadAll is false
        // continue to read next pkt data
        return 0;
    }
/*
    int HttpConnection::process(CHttpParserWrapper &http_parse, HTTPResponse &http_response)
    {
        std::string url =  m_http_parser.GetUrl();           
        LOG_DEBUG("We have got http whole pkt now. Parser url %s", url.c_str());
 
        //based on usr to call process_request
        return 0;
    }

    int HttpConnection::handle_msg_serv_request()
    {
        // json
        
        // OK:
        //
        // code = 0 msg = ""  
        //      priorIP = "192.168.1.2" 
        //      backupIP = "192.168.1.3"
        //      port     = "port"
        //
        //      msfsPrior = "http://127.0.0.1:6080/"
        //      msfsBackup = "http://127.0.0.1:6090/"
        //      discovery  = "http://ip:port"/
        // 
        // FAILD:
        //
        // code = 1 msg = "none msg_server"
        // code = 2 msg = "all msg_server are overload"
        
        return 0;
    }
    */

    int HttpConnection::http_send(HTTPResponse &res)
    {
        uint32_t length = res.body.str().length();
        
        CMbuf_ptr mbuf = make_shared<CMbuf>(length + 256); // 256 for header
        
        /* build http response */
        int header_len = 
            snprintf((char *)mbuf->write_ptr(),
                mbuf->available_buf(),
                "HTTP/1.0 %d %s\r\n" \
                "Server: %s %s\r\n" \
                "Date: %s\r\n" \
                "Content-Type: %s\r\n" \
                "Content-Length: %d\r\n\r\n",
                res.code, res.phrase.c_str(),
                "HttpServer", "1.0",
                res.date.c_str(),
                res.type.c_str(),
                length
                );

        mbuf->write_ptr(header_len);
        
        mbuf->copy((const uint8_t *)res.body.str().c_str(), length);
        
        return this->put(mbuf);
    }
}

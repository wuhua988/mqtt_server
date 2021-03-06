#ifndef _reactor_tcp_socket_h_
#define _reactor_tcp_socket_h_

#include "common/mbuf.hpp"
#include "reactor/sock_base.hpp"
#include "reactor/reactor.hpp"
#include <list>

extern int my_printf(const char *fmt, ...);

namespace reactor
{
    class CPoller;

    class CEventHandler : public CSockBase
    {
        public:

            CEventHandler(CReactor *reactor = CReactor::instance());

            virtual int open(void *data = nullptr);
            virtual int close();

            int put(CMbuf_ptr &mbuf);

            virtual int handle_input(socket_t UNUSED(socket))
            {
                LOG_TRACE_METHOD(__func__);
                return 0;
            }

            // use define notify methord
            virtual int notify()
            {
                return 0;
            }

            virtual int handle_output(socket_t socket);
            virtual int handle_exception(socket_t UNUSED(socket))
            {
                LOG_TRACE_METHOD(__func__);
                return 0;
            }

            virtual int handle_timeout(uint32_t UNUSED(time), void *UNUSED(data) = nullptr)
            {
                return 0;
            }

            virtual int handle_close(socket_t socket = INVALID_SOCKET);

            int write_n(uint8_t *buf, uint32_t len);

            CReactor *reactor()
            {
                return m_reactor_ptr;
            }

            uint32_t get_cur_event_mask()
            {
                LOG_TRACE_METHOD(__func__);

                return m_current_event_mask;
            }

            void set_cur_event_mask(uint32_t event_mask)
            {
                LOG_TRACE_METHOD(__func__);

                m_current_event_mask = event_mask;
            }

            std::string &peer_addr()
            {
                return m_str_peer_addr;
            }

            // for socket stat
            std::time_t accept_time()
            {
                return m_accept_time;
            }

            int set_non_block()
            {
                int flags = 0;
                int res = -1;

                flags = fcntl (m_sock_handle, F_GETFL, 0);
                if (flags == -1)
                {
                    return -1;
                }

                flags |= O_NONBLOCK;
                res = fcntl (m_sock_handle, F_SETFL, flags);
                if (res == -1)
                {
                    return -1;
                }

                return 0;
            }

        protected:
            void get_peer_name();

        protected:
            virtual ~CEventHandler();
            int send(CMbuf_ptr &mbuf, int &my_errno);
            int nonblk_send(CMbuf_ptr &mbuf);
            int schedule_write();
            int cancel_schedule_write();

            /// todo list
            /*
               int    handle_timeout();
               int    handler_signal();
               int    handle_exception();
               */


        protected:
            CReactor *m_reactor_ptr = nullptr;
            uint32_t m_current_event_mask;
            uint64_t m_recv_bytes;
            uint64_t m_recv_times;

            uint64_t m_send_bytes;
            uint64_t m_send_times;

            uint64_t m_schedule_write_times;
            uint64_t m_cancel_schedule_write_times;

            uint32_t m_max_msg_in_buffer;

            std::list<CMbuf_ptr>  m_send_msg_queue;

            //CMbuf_ptr m_recv_mbuf;

            std::string m_str_peer_addr;
            std::time_t m_accept_time;
    };
}

#endif

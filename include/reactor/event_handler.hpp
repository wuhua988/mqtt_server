#ifndef _reactor_tcp_socket_h_
#define _reactor_tcp_socket_h_

#include "common/mbuf.hpp"
#include "reactor/sock_base.hpp"
#include <list>

extern int my_printf(const char *fmt, ...);

namespace reactor
{  
    class CPoller;

    class CEventHandler : public CSockBase
    {
	public:

	    CEventHandler(CPoller *poller);

	    virtual int open(void *data = nullptr);
	    virtual int close();

	    int put(CMbuf_ptr &mbuf);

	    virtual int handle_input(socket_t UNUSED(socket))
	    {
		LOG_TRACE_METHOD(__func__); 
		return 0;
	    }

	    virtual int handle_output(socket_t socket);

	    virtual int handle_exception(socket_t UNUSED(socket))
	    {
		LOG_TRACE_METHOD(__func__); 
		return 0;
	    }

	    virtual int handle_close(socket_t socket = INVALID_SOCKET);

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
	    CPoller	*m_poller_ptr = nullptr;
	    uint32_t    m_current_event_mask;    // for later use

	    uint64_t	m_recv_bytes;
	    uint64_t	m_recv_times;

	    uint64_t	m_send_bytes;
	    uint64_t	m_send_times;

	    uint64_t	m_schedule_write_times;
	    uint64_t	m_cancel_schedule_write_times;

	    uint32_t	m_max_msg_in_buffer;

	    std::list<CMbuf_ptr>  m_send_msg_queue; 

	    CMbuf_ptr m_recv_mbuf;
    };
}

#endif

#ifndef _reactor_sock_base__h_
#define _reactor_sock_base__h_

#include "reactor/define.hpp"

namespace reactor
{
    class CSockBase
    {
	public:
	    CSockBase() : m_sock_handle(INVALID_SOCKET)
	    {
		LOG_TRACE_METHOD(__func__);
	    }

	    virtual ~CSockBase()
	    {
		LOG_TRACE_METHOD(__func__); 
	    }

	    virtual socket_t get_handle()
	    {
		LOG_TRACE_METHOD(__func__);   
		return m_sock_handle;
	    }

	    virtual void set_handle(socket_t socket_id)
	    {
		LOG_TRACE_METHOD(__func__);   
		m_sock_handle = socket_id;
	    }

	    virtual int close()
	    {
		LOG_TRACE_METHOD(__func__);   
		if (m_sock_handle != INVALID_SOCKET )
		{
		    ::close(m_sock_handle);
		    m_sock_handle = INVALID_SOCKET;
		}
		return 0;
	    }

	protected:
	    socket_t  m_sock_handle;
    };
}

#endif /* _reactor_sock_base__h_ */

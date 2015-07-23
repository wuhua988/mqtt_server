#ifndef _timer_handler_h__
#define _timer_handler_h__

#include "reactor/define.hpp"
#include "reactor/poller_epoll.hpp"
#include "reactor/event_handler.hpp"

namespace reactor
{
    class CTimerHandler : public CEventHandler
    {

	public:
	    CTimerHandler(CPoller *poller): CEventHandler(poller)
	    {
		LOG_TRACE_METHOD(__func__);
	    }
	
	    int open(uint32_t start_second, int interval_second); 
	    virtual int handle_input(socket_t sock_id); 

	private:
	    ~CTimerHandler()
	    {
		LOG_TRACE_METHOD(__func__);   
	    }

	protected:
	    uint32_t    m_start_second;
	    uint32_t    m_interval_second;
    };

}

#endif


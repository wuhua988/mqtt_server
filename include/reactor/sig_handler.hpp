#ifndef _sig_handler_h__
#define _sig_handler_h__

#include "reactor/define.hpp"
#include "reactor/poller_epoll.hpp"
#include "reactor/event_handler.hpp"
#include <set>

namespace reactor
{
    class CSigHandler : public CEventHandler
    {
	public:
	    CSigHandler(CPoller *poller): CEventHandler(poller)
	{
	    LOG_TRACE_METHOD(__func__);
	}

	    int open(std::set<int> &signal_set, std::set<int> &signal_ign_set);

	    virtual int handle_input(socket_t sock_id);

	private:
	    ~CSigHandler()
	    {
		LOG_TRACE_METHOD(__func__); 
	    }

	protected:

	    std::set<int> m_signal_set;
	    std::set<int> m_signal_ign_set;
    };
}

#endif

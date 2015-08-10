#ifndef _reactor_poller_h_
#define _reactor_poller_h_

#include "reactor/define.hpp"
#include "common/mbuf.hpp"
#include <mutex>
#include <deque>
//#include <sys/eventfd.h>

namespace reactor
{
    class CEventHandler;
/*
    class CMessage
    {
	public:
	    enum class MSG_TYPE {PUBLISH_MSG=1};
	    
	    MSG_TYPE  m_msg_type;
	    uint32_t  m_time;
	    uint32_t  reserved;

	    CMbuf_ptr m_buf;
    };
*/
    class CPoller
    {
	public:
	    CPoller(): m_running_flag(true)
	    {
		LOG_TRACE_METHOD(__func__); 
	    };

	    virtual ~CPoller()
	    {
		LOG_TRACE_METHOD(__func__); 
	    };

	    virtual bool run(int32_t tm) = 0; 
	    virtual void stop() = 0;

	    virtual int add_event(CEventHandler *event_handler, uint32_t event_mask) = 0;
	    virtual int del_event(CEventHandler *event_handler, uint32_t event_mask) = 0;
	    virtual int mod_event(CEventHandler *event_handler, uint32_t event_mask) = 0;

	    virtual void regist_notify(CEventHandler *notify) = 0;
	    virtual void unregist_notify() = 0;
	    virtual int notify(CMbuf_ptr &mbuf) = 0;

	    virtual int  pop_front(CMbuf_ptr &msg) = 0;
	    virtual int  push_back(CMbuf_ptr &msg) = 0;

	protected:
	    CEventHandler *m_notify_writer = nullptr;
	    handle_t      m_poller_handle;

	    std::unordered_map<socket_t, CEventHandler *>    m_map_event_handlers;
	    
	    std::mutex					     m_notify_mutex;
	    std::deque<CMbuf_ptr>			     m_notify_deque;

	    bool					     m_running_flag;
    };
}


#endif

#include "reactor/event_handler.hpp"
#include "reactor/poller_epoll.hpp"

namespace reactor
{
    CPollerEpoll::CPollerEpoll()
    {
	LOG_TRACE_METHOD(__func__);
    }

    int CPollerEpoll::open(int UNUSED(max_fd_size))
    {
	LOG_TRACE_METHOD(__func__);

#ifdef EPOLL_CLOEXEC
	this->m_poller_handle = epoll_create1(EPOLL_CLOEXEC);
#else
	this->m_poller_handle = epoll_create(max_fd_size);
#endif
	if (this->m_poller_handle == -1)
	{
	    LOG_ERROR("epoll_create() Error.");
	    return -1;
	}
	
	return 0;
    }

    CPollerEpoll::~CPollerEpoll()
    {
	LOG_TRACE_METHOD(__func__);

	if ( this->m_poller_handle != -1 )
	{
	    this->close();
	}
    }

    int CPollerEpoll::close()
    {
	LOG_TRACE_METHOD(__func__);

	int count = 0;
	LOG_DEBUG("In CPollerEpoll::close(), map size %d", (int) m_map_event_handlers.size());

	for (auto it = m_map_event_handlers.begin(); it != m_map_event_handlers.end();it = m_map_event_handlers.begin())
	{
	    count++;
	    auto last_pos = it;
	    // ++it; // avoid iterator failes

	    LOG_DEBUG("Clean %d clients, handle %d", count, it->second->get_handle());
	    last_pos->second->handle_close();
	}

	LOG_DEBUG("Clean %d clients", count);

	::close(this->m_poller_handle);
	this->m_poller_handle = -1;

	return 0;
    }

    void CPollerEpoll::stop()
    {
	m_running_flag = false;
    }

    /* for Linux */
    bool CPollerEpoll::run(int32_t timeout)
    {
	LOG_TRACE_METHOD(__func__);

	if (!m_running_flag)
	{
	    return false;
	}

	int32_t result = epoll_wait(this->m_poller_handle, this->m_poller_events, MAX_EVENT_SIZE, timeout < 0 ? INFINITE : timeout);
	if (result <  0)
	{
	    if (errno == EINTR)
	    {
		return true;
	    }
	    else
	    {
		LOG_ERROR("epoll_wait end,errno = %d, %s", errno, strerror(errno));
		return false;
	    }
	}

	// timeout
	if (result == 0)
	{
	    return true;
	}

	for(int32_t i = 0; i < result; i++)
	{
	    uint32_t  event = this->m_poller_events[i].events;
	    CEventHandler *event_handler = (CEventHandler *)this->m_poller_events[i].data.ptr;

	    if (event_handler == nullptr)
	    {
		LOG_WARN("eventm_poller_handle is nullptr.");
		continue;
	    }

	    int sock_id = event_handler->get_handle();

	    if (event & EPOLLIN)
	    {
		if (event_handler->handle_input(sock_id) < 0)
		{
		    LOG_INFO("In epoll run, handle_input return < 0, call handle_close now");

		    event_handler->handle_close(sock_id);
		    continue; // we just ready to call close now, no need further more
		}
	    }

	    if (event & EPOLLOUT)
	    {
		if (event_handler->handle_output(sock_id) < 0)
		{
		    event_handler->handle_close(sock_id);
		}
		continue; // we just ready to call close now, no need further more

	    }

	    // error
	    if (event & EPOLLERR)
	    {
		event_handler->handle_close(sock_id);
	    }
	}

	return true;
    }

    int CPollerEpoll::add_event(CEventHandler *event_handler, uint32_t event_mask)
    {
	// 检查 tcp_socket and  socket_t 句柄
	LOG_TRACE_METHOD(__func__);

	socket_t socket_id = event_handler->get_handle();
	struct epoll_event ev;

	ev.events = this->convert_event_mask(event_mask);
	ev.data.u64 = 0;
	ev.data.ptr = (void*)event_handler;

	LOG_DEBUG("In add_event, ev.events 0x%x, sock_id %d",ev.events, socket_id);

	if (epoll_ctl(this->m_poller_handle, EPOLL_CTL_ADD, socket_id, &ev) < 0)
	{
	    LOG_ERROR("epoll_ctl EPOLL_CTL_ADD error! %d, %s", errno, strerror(errno));
	    return -1;
	}

	// put it into hash_map
	auto it = this->m_map_event_handlers.find (socket_id);
	if ( it == this->m_map_event_handlers.end())
	{
	    this->m_map_event_handlers[socket_id] = event_handler;
	}
	else
	{
	    if ( it->second == event_handler )
	    {
		// repeat add the same socket
		return 0;
	    }
	    else
	    {
		// alreay have tcp_socket, but not the same
		LOG_WARN("add_event, socket_id already exists, but not the same tcp socket. Something wrong?");
	    }
	}

	event_handler->set_cur_event_mask(event_mask);

	return 0;

    }

    int CPollerEpoll::del_event(CEventHandler *event_handler, uint32_t UNUSED(event_mask))
    {
	LOG_TRACE_METHOD(__func__);

	socket_t socket_id = event_handler->get_handle();

	int result = epoll_ctl(this->m_poller_handle, EPOLL_CTL_DEL, socket_id, 0);
	if(result < 0)
	{
	    LOG_ERROR("epoll_ctl remove error %s, %d", strerror(errno), socket_id);
	    return -1;
	}

	event_handler->set_cur_event_mask(0);

	// delete it from  hash_map
	this->m_map_event_handlers.erase(socket_id);
	return 0;
    }


    int CPollerEpoll::mod_event(CEventHandler *event_handler, uint32_t event_mask)
    {
	LOG_TRACE_METHOD(__func__);

	socket_t socket = event_handler->get_handle();
	struct epoll_event ev;

	ev.events = this->convert_event_mask(event_mask);
	ev.data.u64 = 0;
	ev.data.ptr = (void*)event_handler;

	LOG_DEBUG("CPollerEpoll::mod_event,  change ev.events to 0x%x", ev.events);

	// change EPOLL_CTL_MOD to ADD for performace. see libevent2 ???
	if (epoll_ctl(this->m_poller_handle, EPOLL_CTL_MOD, socket, &ev) < 0)
	{
	    LOG_ERROR("epoll_ctl EPOLL_CTL_MODE error!, errno %d, %s", errno, strerror(errno));
	    return -1;
	}

	event_handler->set_cur_event_mask(event_mask);

	return 0;
    }

    void CPollerEpoll::regist_notify(CEventHandler *notify)
    {
	this->m_notify_writer = notify;
    }

    void CPollerEpoll::unregist_notify()
    {
	this->m_notify_writer = nullptr;
    }

    int CPollerEpoll::notify(CMbuf_ptr &mbuf)
    {	
	std::lock_guard<std::mutex> guard(m_notify_mutex);

	
	if (this->m_notify_writer == nullptr)
	{
	    LOG_DEBUG("No notify registed in epoll");
	    return -1;
	}

	LOG_DEBUG("In CPollEpoll::notify() push msg to queue and notify");

	m_notify_deque.push_back(mbuf);

	return this->m_notify_writer->notify();
	
    }

    int CPollerEpoll::pop_front(CMbuf_ptr &msg)
    {
	if (m_notify_deque.empty())
	{
	    return -1;
	}

	msg = m_notify_deque.front();
	m_notify_deque.pop_front();
	
	return 0;
    }

    int CPollerEpoll::push_back(CMbuf_ptr &msg)
    {
	m_notify_deque.push_back(msg);
	return 0;
    }

    uint32_t CPollerEpoll::convert_event_mask(uint32_t e)
    {
	LOG_TRACE_METHOD(__func__);

	uint32_t op = 0;
	if(e & EVENT_READ) op |=  EPOLLIN;
	if(e & EVENT_WRITE) op |= EPOLLOUT;
	if(e & EVENT_ERROR) op |= EPOLLERR;
	return op;
    }
}

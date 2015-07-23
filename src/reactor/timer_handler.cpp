#include "reactor/timer_handler.hpp"

#include <sys/timerfd.h>    

namespace reactor
{

    int CTimerHandler::open(uint32_t start_second, int interval_second)
    {
	this->m_start_second = start_second;
	this->m_interval_second = interval_second;

	if((m_sock_handle = timerfd_create(CLOCK_MONOTONIC,TFD_NONBLOCK|TFD_CLOEXEC)) < 0)
	{
	    LOG_ERROR("Timer fd create failed. %d, %s", errno, strerror(errno));
	    return -1;
	}

	struct itimerspec new_value;
	// struct itimerspec old_value;
	// bzero(&new_value, sizeof(new_value));
	// bzero(&old_value,sizeof(old_value));

	struct timespec start, interval;
	start.tv_sec = start_second;
	start.tv_nsec = 0;

	interval.tv_sec = interval_second;
	interval.tv_nsec = 0;

	new_value.it_value = start;
	new_value.it_interval = interval;

	if( timerfd_settime(m_sock_handle, 0, &new_value, NULL) < 0) // 0 relative time
	{
	    LOG_ERROR("Settime error, %d, %s\n", errno, strerror(errno));
	    return -1;
	}

	return CEventHandler::open();
    }

    int CTimerHandler::handle_input(socket_t)
    {
	uint64_t times = 0;
	int res = read(m_sock_handle, &times,sizeof(times));
	if (res > 0)
	{
	    // epoll->check_timeout; check idle client
	    LOG_DEBUG("In CTimerHandler::handle_input");
	}

	return 0;
    }

}


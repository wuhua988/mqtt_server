#ifndef _REACTOR_COMMON_DEFINE_
#define _REACTOR_COMMON_DEFINE_


#include <algorithm>
#include <atomic>
#include <thread>
#include <mutex>
#include <string.h>
#include <iostream>
#include <fstream>
#include <memory>
#include <functional>
#include <chrono>
#include <map>
#include <unordered_map>
#include <queue>
#include <iterator>
#include <vector>

#include "pre_define.hpp"

#define _HAS_LOG4CPLUSH_LOG_

#ifdef _HAS_LOG4CPLUSH_LOG_
    #include "common/my_logger.h"
#else
    int my_printf(const char *fmt, ...);
    #define LOG_DEBUG my_printf                                                                         
    #define LOG_WARN  my_printf                                                                         
    #define LOG_ERROR my_printf                                                                         
    #define LOG_TRACE_METHORD my_printf                                                                 
#endif

#ifdef UNUSED
#elif defined(__GNUC__)
# define UNUSED(x) UNUSED_ ## x __attribute__((unused))
#elif defined(__LCLINT__)
# define UNUSED(x) /*@unused@*/ x
#else
# define UNUSED(x) x
#endif

// void dcc_mon_siginfo_handler(int UNUSED(whatsig)) 


/* IOCP */
#if defined(HAVE_WINDOWS)

#define HAVE_IOCP
#include <WinSock2.h>
#include <Windows.h>
#include <MSWSock.h>

#pragma comment(lib, "ws2_32")
#pragma comment(lib, "Mswsock")

#endif

/* EPOLL */
#if defined(HAVE_LINUX) || defined(HAVE_ANDROID)
#ifndef HAVE_EPOLL
#define HAVE_EPOLL
#endif // !HAVE_EPOLL
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#define gettid() syscall(__NR_gettid)
#endif

/* KQUEUE */
#if defined(HAVE_FREEBSD) \
    || defined(HAVE_KFREEBSD) \
|| defined(HAVE_NETBSD) \
|| defined(HAVE_OPENBSD) \
|| defined(HAVE_DARWIN)

#define HAVE_KQUEUE

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/event.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#endif

/* NOT SUPPORT */
#if !defined(HAVE_IOCP) \
    && !defined(HAVE_EPOLL) \
&& !defined(HAVE_KQUEUE)
#error Operating system does not support(pre_define.hpp).
#endif

/* define some ... */
#define ACCPET_THREAD 2
#define MAX_MESSAGE_LEGNTH 20480
#define MAX_SOCKET 65535
#define MAX_EVENT_SIZE 256
#define CLIENT_MAX_SEND_BUFFER_SIZE 1024*1024*10
#define CLIENT_MAX_RECV_BUFFER_SIZE 1024*1024*10
#define SERVER_MAX_SEND_BUFFER_SIZE 1024*1024*1024
#define SERVER_MAX_RECV_BUFFER_SIZE 1024*1024*1024
#define MAX_RECEIVE_BUFFER 10240
#define MAX_SEND_SIZE 655360

#define debug_message_box

enum CONNECT_STATUS
{
    CLIENT_UNCONNECTED = 0,
    CLIENT_CONNECTING,
    CLIENT_CONNECTED
};

#ifndef INFINITE
#define INFINITE			0xFFFFFFFF
#endif
//
#ifndef INVALID_SOCKET
#define  INVALID_SOCKET -1
#endif

#ifndef SOCKET_ERROR
#define SOCKET_ERROR		-1
#endif

#ifdef HAVE_ANDROID
#define EPOLLONESHOT		0x40000000
#endif

/* type */
#ifdef HAVE_IOCP
typedef SOCKET socket_t;
typedef HANDLE handle_t;
// 		typedef WSABUF buffer_t;
typedef int socklen_t;
#else
typedef int socket_t;
typedef int handle_t;
// 		typedef struct {
// 			ulong_t len;
// 			char* buf;
// 		} buffer_t;
#endif

namespace reactor
{     
    typedef uint32_t EVENT_T;

    /* 目前只定义TCP的3个类型 */
    enum IO_TYPE
    {
	HANDLE_ACCEPT,
	HANDLE_RECV,
	HANDLE_SEND,
	HANDLE_COMPLETE,
    };

    enum IO_EVENT_TYPE
    {
	EVENT_NONE = 0,
	EVENT_OPEN = 1 << 0,
	EVENT_READ = 1 << 1,
	EVENT_WRITE = 1 << 2,
	EVENT_CLOSE = 1 << 3,
	EVENT_ERROR = 1 << 4,
	EVENT_TIMEOUT = 1 << 5,
	EVENT_SIGNAL = 1 << 6,
    };

    /* 异步操作带的DATA数据 */
    struct BASE_IO_DATA
    {
	socket_t	socket;
	IO_TYPE		type;
	void*		ptr;
    };

#ifdef HAVE_IOCP

    struct IO_DATA : BASE_IO_DATA, public OVERLAPPED
    {
	DWORD	bytes;
	char	data[MAX_RECEIVE_BUFFER];
    };

#elif defined(HAVE_KQUEUE)

    struct IO_DATA : BASE_IO_DATA
    {
	// TODO
    };

#elif defined(HAVE_EPOLL)

    struct IO_DATA : BASE_IO_DATA
    {
	epoll_event	ev;
    };

#endif

} // reactor namespace

#if defined ( WIN32 )                                                                               
#define __func__ __FUNCTION__                                                                       
#endif  

#define ERROR_RETURN(a,b)\
if ( (a) < 0 )\
{\
    return b;\
}       

#define NULL_PTR_RETRUN(a,b)\
{\
    if ( (a) == nullpr )\
    {\
	return b;\
    }\
}

#define GETSETVAR(type, name)\
protected:\
    type m_##name;\
public:\
    const type & get_##name() const { return m_##name; }\
    void set_##name(const type & newval) { m_##name = newval; }

#endif

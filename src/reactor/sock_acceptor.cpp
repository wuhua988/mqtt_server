#include "reactor/define.hpp"
#include "reactor/sock_acceptor.hpp"

/*
accept4:

#include <sys/types.h>
#include <sys/socket.h>

*/

namespace reactor
{
    CSockAcceptor::CSockAcceptor()
    {
	LOG_TRACE_METHOD(__func__); 
    }

    CSockAcceptor::~CSockAcceptor()
    {
	LOG_TRACE_METHOD(__func__); 
	// ~CSockBase close handle
    }

    int CSockAcceptor::open(const CSockAddress& address, int max_listen)
    {
	LOG_TRACE_METHOD(__func__); 

	this->m_sock_address = address;
	struct sockaddr_in sockAddr = this->m_sock_address.sock_addr();

	this->m_sock_handle = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	int i = 1;
	if(setsockopt(this->m_sock_handle, SOL_SOCKET, SO_REUSEADDR, (void*)&i, sizeof(int)) == -1)
	{
	    LOG_ERROR("Setsockopt error, %s.", strerror(errno));
	    return -1;
	}

	if (::bind(this->m_sock_handle, (struct sockaddr*)&sockAddr, sizeof(struct sockaddr)) == -1)
	{
	    LOG_ERROR("socket bind faild, %s.", strerror(errno));
	    return -1;
	}

	if (::listen(this->m_sock_handle, max_listen) == -1)
	{
	    LOG_ERROR("socket listen faild, %s.", strerror(errno));
	    return -1;
	}

	return 0;

    }

    int CSockAcceptor::close()
    {
	LOG_TRACE_METHOD(__func__); 

	return CSockBase::close();
    }

    socket_t CSockAcceptor::accept(CEventHandler *tcp_socket)
    {
	LOG_TRACE_METHOD(__func__); 

	struct sockaddr_in client_address;
	socklen_t length = sizeof(struct sockaddr_in);
	socket_t socket_id = 0;

	socket_id = ::accept4(this->m_sock_handle, (sockaddr *)&client_address, &length, SOCK_NONBLOCK|SOCK_CLOEXEC);
	if (socket_id < 0 || socket_id == 0)
	{
	    return -1;
	}

	LOG_DEBUG("In CSockAccptor ::accept, return socket id %d", socket_id);


	tcp_socket->set_handle(socket_id);

	LOG_DEBUG("Exit CSockAccptor::accept");


	return socket_id;
    }
}

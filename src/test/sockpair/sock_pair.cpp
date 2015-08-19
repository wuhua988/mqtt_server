#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/syscall.h>
#include <errno.h> 
#include <string.h>

extern int errno;

#ifndef SOCK_NONBLOCK
#define SOCK_NONBLOCK O_NONBLOCK
#endif

int main()
{
    int fds[2];
    if (socketpair(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0, fds) == -1)
    {
	fprintf(stderr, "socketpair(SOCK_NONBLOCK) failed");
	return -1;
    }

    write(fds[0], "hello", 5);

    char buf[32];
    int res = read(fds[0], buf ,32);
    if (res <= 0)
    {
	fprintf(stderr, "read failed %d, %s\n", errno, strerror(errno));
    }

    res = read(fds[1], buf ,32);
    if (res <= 0)
    {
	fprintf(stderr, "read failed %d, %s\n", errno, strerror(errno));   
	return -1;
    }
    buf[res] = 0;

    fprintf(stderr, "Recv from %s\n", buf);

    return 0;
}



#include <sys/signalfd.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>


#include <sys/timerfd.h>                                                                            
#include <sys/time.h>                                                                               
#include <time.h>                                                                                   
#include <unistd.h>                                                                                 
#include <stdlib.h>                                                                                 
#include <stdio.h>                                                                                  
#include <stdint.h>        /* Definition of uint64_t */

#include <sys/epoll.h>
#include <string.h>
#include <memory.h>

#include <errno.h>
extern int errno;


#define handle_error(msg) \
    do { perror(msg); exit(EXIT_FAILURE); } while (0)

int main(int argc, char *argv[])
{
    sigset_t mask;
    int sfd;
    struct signalfd_siginfo fdsi;
    ssize_t s;

    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGQUIT);

    if (sigprocmask(SIG_BLOCK, &mask, NULL) == -1)
	handle_error("sigprocmask");

    sfd = signalfd(-1, &mask, 0);
    if (sfd == -1)
	handle_error("signalfd");

    fprintf(stderr, "sfd %d\n", sfd);

    int efd = epoll_create(256); 
    struct epoll_event ev,events[256];
    
    ev.data.fd = sfd;
    ev.events = EPOLLIN;

    if( epoll_ctl(efd,EPOLL_CTL_ADD,sfd,&ev) < 0)
    {
	printf("epoll_ctl error");
    }

    int num = 0;

    while(1)
    {
	if(( num = epoll_wait(efd,events,256,-1)) > 0)
	{
	    for(int i=0;i<num;i++)
	    {
		if(events[i].data.fd == sfd)
		{
		    s = read(sfd, &fdsi, sizeof(struct signalfd_siginfo)); 
		    if (s != sizeof(struct signalfd_siginfo))    
		    {
			continue;
		    }
		    
		    fprintf(stderr, "Recv signal %d", fdsi.ssi_signo);

		}
	    }
	}

    }

    return 0;
}

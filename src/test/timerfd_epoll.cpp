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

void print_time()                                                                                    
{                                                                                                   
    struct timeval tv;                                                                              
    gettimeofday(&tv, NULL);                                                                        
    printf("printTime:  current time:%ld.%ld\n", tv.tv_sec, tv.tv_usec);                             
}

int main()
{
    int efd = epoll_create(256);             
    // setnonblock(efd);
    struct epoll_event ev,events[256];

    int tfd; //timer fd

    if((tfd= timerfd_create(CLOCK_MONOTONIC,TFD_NONBLOCK)) < 0)
    {
	printf("timer fd create error");
	return -1;
    }

    struct itimerspec newValue;
    struct itimerspec oldValue;
    bzero(&newValue,sizeof(newValue));  
    bzero(&oldValue,sizeof(oldValue));

    struct timespec ts;
    ts.tv_sec = 5;
    ts.tv_nsec = 0;

    /*both interval and value have been set*/
    newValue.it_value = ts; 
    newValue.it_interval = ts;

    if( timerfd_settime(tfd,0,&newValue,&oldValue) <0)
    {
	printf("settime error, %s\n", strerror(errno));
    }   

    ev.data.fd = tfd;
    ev.events = EPOLLIN;

    if( epoll_ctl(efd,EPOLL_CTL_ADD,tfd,&ev) < 0)
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
		if(events[i].data.fd == tfd)
		{
		    uint64_t times = 0;
		    int res = read(tfd, &times,sizeof(times));
		    if (res > 0)
		    {
			printf("%ld\n", times);
			print_time();
		    }
		}
	    }       
	}
    }   

    return 0;
}

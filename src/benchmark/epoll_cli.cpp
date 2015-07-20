#include <iostream>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>

#include <thread>

#include <sys/types.h> 
#include <unistd.h>

#include <unistd.h>
extern char *optarg;
extern int optind, opterr, optopt;

#include <getopt.h>

#include "mqttc++/mqtt_msg.hpp" 

int write_n(int fd, char *buf, ssize_t len);

void print_hex_dump(const uint8_t *buf, int len)
{
    /* char buf[1024]; */
    int i = 0;

    LOG_DEBUG( "0000 ");
    for (; i < len; i++)
    {
	if (i && ((i%16) == 0))
	{
	    LOG_DEBUG("\n%04x ", i);
	}

	LOG_DEBUG( "%02X ", buf[i]);
    }

}

//#define _HAS_LOG4CPLUSH_LOG_
//#include "common/my_logger.h"

using namespace std;

#define MAXLINE 102400
#define OPEN_MAX 100
#define LISTENQ 100
#define SERV_PORT 5000
#define INFTIM 1000

//#include "mqttc++/mqtt_msg.hpp"


int set_nonblock(int fd)
{
    int opts = -1;
    if ((opts = fcntl(fd,F_GETFL)) < 0)
    {
	LOG_ERROR("fcntl F_GETFL failed fd %d", fd);
	return -1;
    }

    opts = opts|O_NONBLOCK;
    if( fcntl(fd,F_SETFL,opts) < 0)
    {
	LOG_ERROR("fcntl F_SETFL failed fd %d", fd);
	return -1;
    }

    return 0;
}

int set_reuse(int fd)
{
    int i = 1;
    if(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (void*)&i, sizeof(int)) == -1)
    {
	LOG_ERROR("setsockopt SO_REUSEADDR failed fd %d", fd);
	return -1;
    }

    return 0;

}

int start_server(int port)
{
    struct sockaddr_in serveraddr;
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);

    // 把socket设置为非阻塞
    set_nonblock(listenfd);

    // 设置reuse
    set_reuse(listenfd);

    bzero(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    const char *local_addr="127.0.0.1";
    inet_aton(local_addr,&(serveraddr.sin_addr));  //htons(portnumber);

    serveraddr.sin_port = htons(port);
    bind(listenfd,(sockaddr *)&serveraddr, sizeof(serveraddr));
    listen(listenfd, LISTENQ);

    LOG_INFO("Server is listen at %d", port);

    return listenfd;
}

int start_client(const char *host, int port, const char *local_host = NULL)
{
    int client_socket = socket(AF_INET,SOCK_STREAM,0);

    LOG_DEBUG("client_socket %d", client_socket);

    if( client_socket < 0)
    {
	LOG_ERROR("Create socket failed, errno %d", errno);
	return -1;
    }

    //设置一个socket地址结构server_addr,代表服务器的internet地址, 端口
    struct sockaddr_in server_addr;
    bzero(&server_addr,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    if(inet_aton(host,&server_addr.sin_addr) == 0)
    {
	LOG_ERROR("Server address inet_aton failed, errno %d!", errno);
	return -1;
    }

    if (local_host != NULL)
    {
	sockaddr_in client_addr;
	client_addr.sin_family = AF_INET;
	client_addr.sin_port = 0;

	client_addr.sin_addr.s_addr = inet_addr(local_host);

	if (bind(client_socket,(struct sockaddr*)&client_addr, sizeof(client_addr)) == -1)
	{
	    LOG_ERROR("Bind client failed, local_host %s, errno %d, %s",
		    local_host, errno, strerror(errno));

	    close(client_socket);
	    return -1;
	}
    }

    server_addr.sin_port = htons(port);
    socklen_t server_addr_length = sizeof(server_addr);

    //向服务器发起连接,连接成功后client_socket代表了客户机和服务器的一个socket连接
    if(connect(client_socket,(struct sockaddr*)&server_addr, server_addr_length) < 0)
    {
	LOG_ERROR("Connect to %s:%d failed! error %d, %s", host, port, errno, strerror(errno));
	close(client_socket);

	return -1;
    }


    // send connect msg.
    char buf[256];
    CMqttFixedHeader fixed_header(MqttType::CONNECT);
    CMqttConnect connect_msg((uint8_t *)buf, 256, fixed_header);

    char client_id[64];
    snprintf(client_id, 64, "%d_%d", getpid(), client_socket);
    connect_msg.client_id((char *)client_id);
    int enc_len = connect_msg.encode();

    // print_hex_dump((const uint8_t*)buf, enc_len);
    if (enc_len > 0)
    {
	write_n(client_socket, buf, enc_len);
    }


    // write subscriber now
    FixHeaderFlag header_flag;
    header_flag.bits.msg_type = (uint8_t)MqttType::SUBSCRIBE;
    CMqttPkt mqtt_pkt((uint8_t *)buf, 128);

    mqtt_pkt.write_byte(header_flag.all);

    uint32_t length = 10;// msg_id(2), topic = hello(5+2), qos(0) 
    uint8_t  remain_length_bytes;

    mqtt_pkt.write_remain_length(length, remain_length_bytes);
    mqtt_pkt.write_short((unsigned short)client_socket);
    mqtt_pkt.write_string((uint8_t *)"hello", 5);
    mqtt_pkt.write_byte(0);

    enc_len = mqtt_pkt.length();

    print_hex_dump((const uint8_t*)buf, enc_len);
    // write subcriber msg
    if (enc_len > 0)
    {
	write_n(client_socket, buf, enc_len);
    }

#if 0
    uint8_t peer0_1[] = {
	0x82, 0x0a, 0x00, 0x01, 0x00, 0x05, 0x68, 0x65, 
	0x6c, 0x6c, 0x6f, 0x01 };

    LOG_INFO("Ready to wirte data len %d",(int) (enc_len + sizeof(peer0_1)));
    memcpy(buf + enc_len, peer0_1, sizeof(peer0_1));

    write_n(client_socket,buf, enc_len + sizeof(peer0_1));
#endif

    return client_socket;
}

int regist_fd(int epfd, int fd, int event)
{
    struct epoll_event ev;
    ev.data.fd = fd;
    ev.events = event;

    // 注册epoll事件
    int res =  epoll_ctl(epfd,EPOLL_CTL_ADD,fd,&ev);

    LOG_DEBUG("epoll_ctl(EPOLL_CTL_ADD) fd %d, event %d, res 0x%x", fd, event, res);

    return res;
}

int unregist_fd(int epfd, int fd)
{
    int res = epoll_ctl(epfd, EPOLL_CTL_DEL, fd, 0);
    LOG_DEBUG("epoll_ctl(EPOLL_CTL_DEL) fd %d, event %d, res 0x%x", fd, 0, res);
    return res;
}

int mod_fd(int epfd, int fd, int event)
{
    struct epoll_event ev;
    ev.data.fd = fd;
    ev.events = event;

    int res = epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev);

    LOG_DEBUG("epoll_ctl(EPOLL_CTL_DEL) fd %d, event %d, res 0x%x", fd, event, res);

    return res;
}

int write_n(int fd, char *buf, ssize_t len)
{
    while (len > 0)
    {

	ssize_t res = write(fd, buf, len);
	if (res < len)
	{
	    if (res == 0)
	    {
		continue;
	    }

	    if (res < 0)
	    {
		if (EAGAIN == errno || EWOULDBLOCK == errno || EINTR == errno )
		{
		    continue;
		}
		else
		{
		    LOG_ERROR("socket(%d) send error, errno %d.", fd, errno);
		    return -1;
		}
	    }
	}

	len -= res;
    }

    return 0;
}

typedef struct
{
    // epoll
    int epfd;

    // 192.168.1.x ->
    char *host;

    char *host_base;
    int start_last_ip;
    int host_size;

    int server_port;
    int host_connect_number;


    int *cli_array;
    int cli_array_size;

    int total_cli_number;
    int max_cli_fd;

}THostCliInfo;

int make_huge_connect(THostCliInfo &host_cli_info)
{
    for (int i = 0; i < host_cli_info.host_size; i++) // 192.168.1.2 - 192.168.1.60, 取 20个
    {
	char host_addr[64];
	snprintf(host_addr, 64, "%s.%d", host_cli_info.host_base, host_cli_info.start_last_ip+i);

	LOG_INFO("Connected to %s:%d starting", host_addr, host_cli_info.server_port);

	for (int j = 0; j < host_cli_info.host_connect_number; j++) // 5w connct for per ip addr
	{
	    int cli_fd = start_client(host_addr, host_cli_info.server_port);
	    if (cli_fd == -1)
	    {
		LOG_INFO("Connected to %s:%d failed, this host current cli %d,All Client %d max_fd %d",
			host_addr,
			host_cli_info.server_port,
			i,
			host_cli_info.total_cli_number,
			host_cli_info.max_cli_fd);



		break; // goto next host_addr
	    }
	    else
	    {
		host_cli_info.total_cli_number++;

		if (cli_fd >= host_cli_info.cli_array_size - 1)
		{
		    LOG_ERROR("Reached max client number %d, now %d. Return ",
			    host_cli_info.cli_array_size,cli_fd);
		    return -1;
		}

		host_cli_info.cli_array[cli_fd] = cli_fd;

		if (cli_fd > host_cli_info.max_cli_fd)
		{
		    host_cli_info.max_cli_fd = cli_fd;
		}

		set_nonblock(cli_fd);

		// 注册listen_fd 到 epoll
		regist_fd(host_cli_info.epfd, cli_fd, EPOLLIN);
	    }

	    if ( host_cli_info.total_cli_number % 10000 == 0)
	    {
		LOG_INFO("Current %d clients connected to server succeed",
			host_cli_info.total_cli_number);
	    }

	    if (j % 10000)
	    {
		std::this_thread::sleep_for(std::chrono::milliseconds(5));
	    }
	}


    }

    return 0;
}

int use_multi_network_connect(THostCliInfo &host_cli_info)
{
    for (int i = 0; i < host_cli_info.host_size; i++) // 192.168.1.2 - 192.168.1.60, 取 20个
    {
	char host_addr[64];
	snprintf(host_addr, 64, "%s.%d", host_cli_info.host_base, host_cli_info.start_last_ip+i+1);

	LOG_INFO("Connected to %s:%d @ ip_addr %s ", host_cli_info.host, host_cli_info.server_port, host_addr);

	for (int j = 0; j < host_cli_info.host_connect_number; j++) // 5w connct for per ip addr
	{
	    int cli_fd = start_client(host_cli_info.host, host_cli_info.server_port, host_addr);
	    if (cli_fd == -1)
	    {
		LOG_INFO("Connected to %s:%d failed, this host current cli %d,All Client %d max_fd %d",
			host_addr,
			host_cli_info.server_port,
			i,
			host_cli_info.total_cli_number,
			host_cli_info.max_cli_fd);



		break; // goto next host_addr
	    }
	    else
	    {
		host_cli_info.total_cli_number++;

		if (cli_fd >= host_cli_info.cli_array_size - 1)
		{
		    LOG_ERROR("Reached max client number %d, now %d. Return ",
			    host_cli_info.cli_array_size,cli_fd);
		    return -1;
		}

		host_cli_info.cli_array[cli_fd] = cli_fd;

		if (cli_fd > host_cli_info.max_cli_fd)
		{
		    host_cli_info.max_cli_fd = cli_fd;
		}

		// 注册listen_fd 到 epoll
		regist_fd(host_cli_info.epfd, cli_fd, EPOLLIN);
	    }

	    if ( host_cli_info.total_cli_number % 10000 == 0)
	    {
		LOG_INFO("Current %d clients connected to server succeed",
			host_cli_info.total_cli_number);
	    }

	    std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
    }

    return 0;
}




static struct option long_options[] = {
    { "help",           no_argument,        NULL,   'h' },
    { "host",           required_argument,  NULL,   's' },
    { "port",           required_argument,  NULL,   'p' },

    { "host_start",     required_argument,  NULL,   't' },
    { "host_num",       required_argument,  NULL,   'n' },
    { "connet_num",     required_argument,  NULL,   'c' },

    { "multi_host",     no_argument,        NULL,   'm' },
    { NULL,             0,                  NULL,    0  }
};

static char short_options[] = "hms:p:n:c:t:";

void usage(int , char* argv[])
{
    fprintf(stderr, "Usage: %s [-h] -s server_ip -p server_port -m [multi_host] -t start_ip -n host_num -c connct_num\n", argv[0]);
    exit(0);
}


int main(int argc, char* argv[])
{
    // open log
    CLoggerMgr logger("log4cplus_cli.properties");

    int server_port = 5050;
    const char *host = "127.0.0.1";

    const char *cli_start_host = "192.168.1.2";
    int multi_host = 0;
    int mulit_host_num = 0;
    int cli_num = 1;

    char line[MAXLINE];
    int read_len = 0;

    // socklen_t clilen;

    opterr = 0;

    for (;;)
    {

	int c; // value;
	c = getopt_long(argc, argv, short_options, long_options, NULL);
	if (c == -1) {
	    /* no more options */
	    break;
	}

	switch (c)
	{
	    case 's':
		host = optarg;
		break;

	    case 'p':
		server_port = atoi(optarg);
		break;

	    case 'm':
		multi_host = 1;
		break;

	    case 't':
		cli_start_host = optarg;
		break;

	    case 'n':
		mulit_host_num =  atoi(optarg);
		break;

	    case 'c':
		cli_num = atoi(optarg);
		break;

	    case '?':
	    case 'h':
		usage(argc, argv);
		break;

	    default:
		usage(argc, argv);
		break;
	}
    }


    // start client connect
    const int MAX_CLIENT = 2000000; // 200w 大小
    int cli_array[MAX_CLIENT];

    if (cli_num > MAX_CLIENT)
    {
	cli_num = MAX_CLIENT;
    }

    for (int i = 0; i < MAX_CLIENT; i++)
    {
	cli_array[i] = -1;
    }



    // 声明epoll_event结构体的变量,ev用于注册事件,数组用于回传要处理的事件
    const int MAX_EVENT_NUM = 1024;
    struct epoll_event events[MAX_EVENT_NUM];

    // 生成用于处理accept的epoll专用的文件描述符
    int epfd = epoll_create1(0);

    int max_fd = -1;

    THostCliInfo hosts_cli_info;

    if (multi_host)
    {
	LOG_INFO("----- We are going to start multi-host mode -----");

	// split host and add
	int host_index = strlen(cli_start_host) - 1;

	int last_ip_num = 0;
	while( host_index > 0)
	{
	    if (cli_start_host[host_index] == '.') // find last .
	    {
		last_ip_num = atoi(cli_start_host+host_index+1);
		break;
	    }

	    host_index--;
	}


	char host_base[64];
	bzero(host_base, 64);
	memcpy(host_base, cli_start_host, host_index);


	LOG_INFO("Muli-Mode Host %s, base_host %s, last ip %d,  host number %d, Connections %d PerHost",
		host, host_base, last_ip_num, mulit_host_num, cli_num);

	hosts_cli_info.epfd = epfd;
	hosts_cli_info.host = (char *)host;

	hosts_cli_info.host_base = (char *)host_base;
	hosts_cli_info.start_last_ip = last_ip_num;
	hosts_cli_info.host_size = mulit_host_num; // 192.168.1.2 -> 192.168.1.7

	hosts_cli_info.server_port = server_port;
	hosts_cli_info.host_connect_number = cli_num;

	hosts_cli_info.cli_array = cli_array;
	hosts_cli_info.cli_array_size = MAX_CLIENT;

	hosts_cli_info.total_cli_number = 0;
	hosts_cli_info.max_cli_fd = 0;

	//make_huge_connect(hosts_cli_info);
	use_multi_network_connect(hosts_cli_info);

	max_fd = hosts_cli_info.max_cli_fd;

	LOG_INFO("All client connect to server %d", hosts_cli_info.total_cli_number);

    }
    else // start one host mode
    {
	LOG_INFO("----- We are going to start signal host mode -----");
	for (int i = 0; i < cli_num; i++)
	{
	    int cli_fd = start_client(host, server_port);
	    if (cli_fd == -1)
	    {
		LOG_INFO("Connected failed %d, max_fd %d", i, max_fd);
		LOG_INFO("Current %d clients connected to server succeed", i);

		break; // 我们只处理已经连接成功的句柄

		for (int j = 0; j < i; j++)
		{
		    int tmp_fd = cli_array[j];
		    if (tmp_fd != -1)
		    {
			unregist_fd(epfd, tmp_fd);
			close(tmp_fd);
		    }
		}

		return -1;
	    }
	    else
	    {
		cli_array[cli_fd] = cli_fd;

		if (cli_fd > max_fd)
		{
		    max_fd = cli_fd;
		}

		cli_num = i + 1;

		// 注册listen_fd 到 epoll
		regist_fd(epfd, cli_fd, EPOLLIN);
	    }

	    if ( i % 10000 == 0)
	    {
		LOG_INFO("Current %d clients connected to server succeed", i+1);
	    }

	    std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	LOG_INFO("All client connect to server %d", cli_num);
    }
    // end for client



    srandom(time(NULL));


    for ( ; ; )
    {
	//等待epoll事件的发
	int nfds = epoll_wait(epfd,events,MAX_EVENT_NUM,500); // 500ms
	// LOG_DEBUG("epoll_wait res %d, errno %d", nfds, errno);

	if (nfds <  0)
	{
	    if (errno == EINTR)
	    {
		LOG_DEBUG("epoll_wait res %d, errno EINTR, continue", nfds);
		continue;
	    }
	    else
	    {
		LOG_ERROR("epoll_wait res %d, errno %d, end", nfds, errno);
		return -1;
	    }
	}

#if 0
	if (nfds == 0)
	{
	    //LOG_DEBUG("epoll_wait res %d, Timeout, continue", nfds);
	    // random sent data to server

	    int rand_sock = random()%max_fd; // max_fd

	    static ssize_t str_len = strlen("Hello");
	    const char *hello = "Hello";

	    if (cli_array[rand_sock] != -1)
	    {
		LOG_DEBUG("Timeout Write data to sockfd %d, actual sockfd %d", rand_sock, cli_array[rand_sock]);
		write_n(rand_sock, (char *)hello, str_len);
	    }
	    //            ssize_t res = write(rand_sock, "Hello", str_len);
	    //            if (res != str_len)
	    //            {
	    //                LOG_ERROR("Write to sockfd [%d] failed. res %d, str_len %d, errno %d",
	    //                                                            rand_sock, res, errno);
	    //            }

	    continue;
	}
#endif

	// 处理所发生的所有事件
	for(int i = 0; i < nfds; ++i)
	{
	    /*
	       if(events[i].data.fd == listen_fd) // 如果新监测到一个SOCKET用户连接到了绑定的SOCKET端口，建立新的连接。
	       {
	       socklen_t clilen = 0;
	       struct sockaddr_in clientaddr;

	       int connfd = accept(listen_fd,(sockaddr *)&clientaddr, &clilen);
	       LOG_DEBUG("Accept res %d", connfd);

	       if (connfd < 0 )
	       {
	       LOG_ERROR("Someting wrong on accept errno %d, end", errno);
	       return -1;
	       }

	       set_nonblock(connfd);

	       char *str = inet_ntoa(clientaddr.sin_addr);
	       LOG_INFO("A new client from %s", str);

	       regist_fd(epfd, connfd, EPOLLIN);
	       }
	       else 
	       */

	    if(events[i].events & EPOLLIN) // 如果是已经连接的用户，并且收到数据，那么进行读入
	    {
		int sockfd = -1;
		if ((sockfd = events[i].data.fd) < 0)
		{
		    LOG_ERROR("EPOLLIN fd < 0 (%d) someting wrong ", sockfd);
		    continue;
		}

		LOG_DEBUG("EPOLLIN fd %d ", sockfd);

		if ((read_len = read(sockfd, line, MAXLINE)) <= 0)
		{
		    unregist_fd(epfd, sockfd);
		    LOG_INFO("Close fd %d, read res %d", sockfd, read_len);

		    cli_array[sockfd] = -1;
		    close(sockfd);
		    continue;

		}

		line[read_len] = '\0';

		LOG_DEBUG("EPOLLIN fd %d, read %s", sockfd, line);

		// mod_fd(epfd, sockfd, EPOLLOUT);

	    }
	    else if(events[i].events & EPOLLOUT) // 如果有数据发送  not reached
	    {
		int sockfd = -1;
		if ((sockfd = events[i].data.fd) < 0)
		{
		    LOG_ERROR("EPOLLOUT fd < 0 (%d) someting wrong ", sockfd);
		    continue;
		}

		LOG_DEBUG("EPOLLOUT fd %d, write %s ", sockfd, line);

		// 判断返回值
		int write_len = write(sockfd, line, read_len);
		if (write_len != read_len)
		{
		    LOG_ERROR("Write data failed, ready to write %d, actually wirte %d", read_len, write_len);
		}

		mod_fd(epfd, sockfd, EPOLLIN);
	    }
	}
    }

    for (int i = 0; i < max_fd; i++)
    {
	if (cli_array[i] != -1)
	{
	    close(cli_array[i]);
	    cli_array[i] = -1;
	}
    }


    return 0;
}

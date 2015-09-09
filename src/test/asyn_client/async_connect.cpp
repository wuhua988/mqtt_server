/* 
 * File:   main.cpp
 * Created on March 7, 2013, 5:54 PM
 */

#include <cstdlib>
#include <string>
#include <iostream>

#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#include <error.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
 #include <string.h>
#include <unistd.h>
#include <stdio.h>

using namespace std;

struct so {
    int fd;
    string val;
};

int select_version(int *fd) {
    int c_fd = *fd;
    fd_set rset, wset;
    struct timeval tval;
    FD_ZERO(&rset);
    FD_SET(c_fd, &rset);
    wset = rset;
    tval.tv_sec = 0;
    tval.tv_usec = 300 * 1000; //300毫秒
    int ready_n;
    if ((ready_n = select(c_fd + 1, &rset, &wset, NULL, &tval)) == 0) {
        close(c_fd); /* timeout */
        errno = ETIMEDOUT;
        perror("select timeout.\n");
        return (-1);
    }
    if (FD_ISSET(c_fd, &rset)) {
        int error;
        socklen_t len = sizeof (error);
        if (getsockopt(c_fd, SOL_SOCKET, SO_ERROR, &error, &len) < 0) {
            cout << "getsockopt error." << endl;
            return -1;
        }
        cout << "in fire." << error << endl;
    }
    if (FD_ISSET(c_fd, &wset)) {
        int error;
        socklen_t len = sizeof (error);
        if (getsockopt(c_fd, SOL_SOCKET, SO_ERROR, &error, &len) < 0) {
            cout << "getsockopt error." << endl;
            return -1;
        }
        cout << "out fire." << error << endl;
    }
    return 0;
}

int epoll_version(int *fd) {
    int c_fd = *fd;
    int ep = epoll_create(1024);
    struct epoll_event event;
    event.events = (uint32_t) (EPOLLIN | EPOLLOUT| EPOLLERR);
    struct so _data;
    _data.fd = c_fd;
    _data.val = "test";
    event.data.ptr = (void*) &_data;
    epoll_ctl(ep, EPOLL_CTL_ADD, c_fd, &event);
    struct epoll_event eventArr[1000];
    int status, err;
    socklen_t len;
    err = 0;
    len = sizeof (err);
    int n = epoll_wait(ep, eventArr, 20, 300);
    for (int i = 0; i < n; i++) {
        epoll_event ev = eventArr[i];
        int events = ev.events;
        if (events & (EPOLLHUP | EPOLLERR)) {
            struct so* so_data = (struct so*) ev.data.ptr;
            cout << so_data->val << ",err event fire." << endl;
            status = getsockopt(c_fd, SOL_SOCKET, SO_ERROR, &err, &len);
            cout << status << "," << err << " : " << strerror(err) << endl;
        }

        if (events & EPOLLIN) {
            struct so* so_data = (struct so*) ev.data.ptr;
            cout << so_data->val << ",in event fire. status and err" << endl;
            status = getsockopt(c_fd, SOL_SOCKET, SO_ERROR, &err, &len);
            cout << status << "," << err << endl;
        }
        if (events & EPOLLOUT) {
            struct so* so_data1 = (struct so*) ev.data.ptr;
            cout << so_data1->val << ",out event fire." << endl;
        }
    }

}

int main(int argc, char** argv) {
    string ip = "127.0.0.1";
    int port = 25698;
    int c_fd, flags, ret;
    struct sockaddr_in s_addr;
    memset(&s_addr, 0, sizeof (s_addr));
    s_addr.sin_family = AF_INET;
    s_addr.sin_port = htons(port);
    s_addr.sin_addr.s_addr = inet_addr(ip.c_str());

    if ((c_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("create socket fail.\n");
        exit(0);
    }
    flags = fcntl(c_fd, F_GETFL, 0);
    if (flags < 0) {
        perror("get socket flags fail.\n");
        return -1;
    }

    if (fcntl(c_fd, F_SETFL, flags | O_NONBLOCK) < 0) {
        perror("set socket O_NONBLOCK fail.\n");
        return -1;
    }
    ret = connect(c_fd, (struct sockaddr*) &s_addr, sizeof (struct sockaddr));
    while (ret < 0) {
        if (errno == EINPROGRESS) {
            break;
        } else {
            perror("connect remote server fail.\n");
            printf("%d\n", errno);
            exit(0);
        }
    }
    //select_version(&c_fd);
    //
    int i = 0;
    while(1)
    {
        if (i++ > 3)
        {
            break;
        }

        epoll_version(&c_fd);
    }
    return 0;
}

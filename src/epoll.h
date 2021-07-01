#ifndef EPOLL_H
#define EPOLL_H

#include <sys/epoll.h>

#define MAXEVENTS 1024

int lan_epoll_create(int flags);
void lan_epoll_add(int epfd, int fs, struct epoll_event *event);
void lan_epoll_mod(int epfd, int fs, struct epoll_event *event);
void lan_epoll_del(int epfd, int fs, struct epoll_event *event);
int lan_epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);

#endif

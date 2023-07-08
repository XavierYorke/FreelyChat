/*
 * @Author  :   XavierYorke 
 * @Contact :   mzlxavier1230@gmail.com
 * @Time    :   2023-07-08
 */

#include "epoller.h"

Epoller::Epoller(int maxEvent): epollfd(epoll_create(1)), events(maxEvent) {
    assert(epollfd != -1 && events.size() > 0);
}

Epoller::~Epoller() {
    close(epollfd);
}

bool Epoller::AddFd(int fd, uint32_t events) {
    if (fd < 0) return false;
    epoll_event ev = {0};
    ev.events = events;
    ev.data.fd = fd;
    return epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev) == 0;
}

bool Epoller::DelFd(int fd) {
    if (fd < 0) return false;
    epoll_event ev = {0};
    return epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, &ev) == 0;
}

bool Epoller::ModFd(int fd, uint32_t events) {
    if (fd < 0) return false;
    epoll_event ev = {0};
    ev.events = events;
    ev.data.fd = fd;
    return epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &ev) == 0;
}

int Epoller::Wait() {
    return epoll_wait(epollfd, &events[0], static_cast<int>(events.size()), -1);
}

int Epoller::GetEventFd(size_t fd) const {
    assert(fd < events.size() && fd >= 0);
    return events[fd].data.fd;
}

uint32_t Epoller::GetEvent(size_t fd) const {
    assert(fd < events.size() && fd >= 0);
    return events[fd].events;
}
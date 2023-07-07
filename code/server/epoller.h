#pragma once
#include <sys/epoll.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <vector>

class Epoller {
public:
    explicit Epoller(int maxEvent);
    ~Epoller();

    bool AddFd(int fd, uint32_t events);
    bool DelFd(int fd);
    bool ModFd(int fd, uint32_t events);

    int Wait();

    int GetEventFd(size_t fd) const;
    uint32_t GetEvent(size_t fd) const;

private:
    int epollfd;
    std::vector<struct epoll_event> events;
};
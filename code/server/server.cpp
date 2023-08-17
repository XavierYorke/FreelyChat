/*
 * @Author  :   XavierYorke 
 * @Contact :   mzlxavier1230@gmail.com
 * @Time    :   2023-07-08
 */

#include "server.h"

#include <assert.h>
#include <iostream>
#include <arpa/inet.h>
#include <filesystem>

using namespace std;

#define STATE_LEN 5

int Server::CurUserCnt = 0;

const static char* Y = "OK";
const static char* N = "NO";

Server::Server()
    : pool(new ThreadPool()),
      epoller(new Epoller(MaxClientNum)),
      ServerPort(1230),
      ServerIP("47.109.81.162"),
      ListenEvent(EPOLLRDHUP | EPOLLET),
      ConnEvent(EPOLLRDHUP | EPOLLET | EPOLLONESHOT),
      manager(new User_manager()) {

    filesystem::path currPath =  filesystem::current_path() / "logs";
    this->logger = make_unique<Logger>(Logger::BOTH, Logger::INFO, currPath);
    
    SockInit();

    string msg = string("FreelyChat Start Successfully!\n");
    logger->Info(msg);
    msg = "Join The FreelyChat By Connecting to " + ServerIP;
    logger->Info(msg);
    // fflush(stdout);
}

Server::~Server() {
    close(lfd);
}

void Server::HandleConnect() {
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    int connfd = accept(lfd, (struct sockaddr*)& addr, &len);
    if (connfd < 0) {
        logger->Error("errno: " + errno);
        return;
    }
    if (CurUserCnt >= MaxClientNum) {
        logger->Info("full connection");
        close(connfd);
    }
    epoller->AddFd(connfd, EPOLLIN | ConnEvent);
    SetNonBlocking(connfd);

    char ClientIP[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &addr.sin_addr, ClientIP, INET_ADDRSTRLEN);
    logger->Info((string)ClientIP + " connected");
    ++CurUserCnt;

    Client client;
    client.sockfd = connfd;
    Clients[connfd] = client;
    pool->addTask(bind(&Server::HandleStart, this, connfd));

}

void Server::HandleStart(int fd) {
    char state[STATE_LEN] = {0};
    while (true) {
        memset(state, 0, STATE_LEN);
        int recv_len = recv(fd, state, STATE_LEN, 0);
        if (recv_len < 0) 
            continue;
        if (recv_len == 0) {
            pool->addTask(bind(&Server::CloseConn, this, fd));
            return;
        }
        
        if (strcmp(state, "1") == 0) {
            pool->addTask(bind(&Server::HandleLogin, this, fd));
            return;
        }

        else {
            pool->addTask(bind(&Server::HandleRegister, this, fd));
            return;
        }
    }
}

void Server::HandleLogin(int fd) {
    char buf[BufferLen] = {0};
    vector<User> user;

    // name
    while (true) {
        while (true) {
            memset(buf, 0, BufferLen);
            int len = recv(fd, buf, BufferLen, 0);
            if (len < 0) continue;
            else if (len == 0) {
                pool->addTask(bind(&Server::CloseConn, this, fd));
                return;
            }
            else break;
        }
        if (strcmp(buf, "Q") == 0 || strcmp(buf, "q") == 0) {
            pool->addTask(bind(&Server::HandleStart, this, fd));
            return;
        }
        else {
            user = manager->m_query(string("where name = '") + buf + string("'"));
            if (user.empty()) {
                send(fd, N, strlen(N), 0);
            }
            else {
                send(fd, Y, strlen(Y), 0);
                break;
            }
        }
    }

    Clients[fd].name = buf;

    // password
    while (true) {
        while (true) {
            memset(buf, 0, BufferLen);
            int len = recv(fd, buf, BufferLen, 0);
            if (len < 0) continue;
            else if (len == 0) {
                pool->addTask(bind(&Server::CloseConn, this, fd));
                return;
            }
            else break;
        }
        if (strcmp(buf, "Q") == 0 || strcmp(buf, "q") == 0) {
            pool->addTask(bind(&Server::HandleStart, this, fd));
            return;
        }
        else {
            if (user[0].m_passwd != buf) {
                send(fd, N, strlen(N), 0);
            }
            else {
                send(fd, Y, strlen(Y), 0);
                break;
            }
        }
    }

    string msg = "Welcome " + Clients[fd].name + ". Online Num: " + to_string(CurUserCnt);
    MsgQ.push(SockMsg(-1, msg));
    pool->addTask(bind(&Server::HandleMsg, this, BROADCAST));
    Clients[fd].isOnline = true;
}

void Server::HandleRegister(int fd) {
    char buf[BufferLen] = {0};
    vector<User> user;

    // name
    while (true) {
        while (true) {
            memset(buf, 0, BufferLen);
            int len = recv(fd, buf, BufferLen, 0);
            if (len < 0) continue;
            else if (len == 0) {
                pool->addTask(bind(&Server::CloseConn, this, fd));
                return;
            }
            else break;
        }
        if (strcmp(buf, "Q") == 0 || strcmp(buf, "q") == 0) {
            pool->addTask(bind(&Server::HandleStart, this, fd));
            return;
        }
        else {
            user = manager->m_query(string("where name = '") + buf + string("'"));
            if (!user.empty()) {
                send(fd, N, strlen(N), 0);
            }
            else {
                send(fd, Y, strlen(Y), 0);
                break;
            }
        }
    }

    Clients[fd].name = buf;

    // password
    while (true) {
        while (true) {
            memset(buf, 0, BufferLen);
            int len = recv(fd, buf, BufferLen, 0);
            if (len < 0) continue;
            else if (len == 0) {
                pool->addTask(bind(&Server::CloseConn, this, fd));
                return;
            }
            else break;
        }
        if (strcmp(buf, "Q") == 0 || strcmp(buf, "q") == 0) {
            pool->addTask(bind(&Server::HandleStart, this, fd));
            return;
        }
        else {
            User user(Clients[fd].name, buf);
            manager->m_insert(user);
            send(fd, Y, strlen(Y), 0);
            break;
        }
    }

    string msg = "Welcome " + Clients[fd].name + ". Online Num: " + to_string(CurUserCnt);
    MsgQ.push(SockMsg(-1, msg));
    pool->addTask(bind(&Server::HandleMsg, this, BROADCAST));
    Clients[fd].isOnline = true;
}

void Server::HandleRecv(Client* client) {
    while (true) {
        if (!client->isOnline) continue;
        char buf[BufferLen] = {0};
        int recv_len = recv(client->sockfd, buf, BufferLen, 0);
        if (recv_len < 0) {
            continue;
        }
        else if (recv_len == 0) {
            pool->addTask(bind(&Server::CloseConn, this, client->sockfd));
            return;
        }

        string msg(buf, recv_len);
        msg = "[" + client->name + "]: " + msg;
        strcpy(buf, msg.c_str());

        MsgQ.push(SockMsg(client->sockfd, buf));
        pool->addTask(bind(&Server::HandleMsg, this, BROADCAST));
    }
}

void Server::HandleMsg(MsgMode mode) {
    SockMsg msg_one = MsgQ.front();
    MsgQ.pop();
    string send_msg = logger->Info(msg_one.msg);

    if (mode == BROADCAST) {
        for (auto& c: Clients) {
            if (c.first != msg_one.sockfd) {
                send(c.first, send_msg.c_str(), send_msg.size(), 0);
            }
        }
    }
    else if (mode == SELF) {
        send(msg_one.sockfd, send_msg.c_str(), send_msg.size(), 0);
    }
}


void Server::Start() {
    while (true) {
        int num = epoller->Wait();
        if ((num < 0) && (errno != EINTR)) {
            logger->Error("epoll error");
            return;
        }

        for (int i = 0; i < num; ++i) {
            int sockfd = epoller->GetEventFd(i);
            uint32_t events = epoller->GetEvent(i);

            if (sockfd == lfd) {
                pool->addTask(bind(&Server::HandleConnect, this));
            }
            else if(events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
                if (!Clients[sockfd].isOnline) continue;
                assert(Clients.count(sockfd) > 0);
                pool->addTask(bind(&Server::CloseConn, this, sockfd));
            }
            else if (events & EPOLLIN) {
                assert(Clients.count(sockfd) > 0);
                pool->addTask(bind(&Server::HandleRecv, this, &Clients[sockfd]));
            }

            else if (events & EPOLLOUT) {
                assert(Clients.count(sockfd) > 0);
            }
        }
    }
}

void Server::SockInit() {
    // create socket
    lfd = socket(AF_INET, SOCK_STREAM, 0);
    assert(lfd != -1);

    // port multiplexing
    int optval = 1;
    assert(setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) !=
           -1);

    // bind
    sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = INADDR_ANY;
    saddr.sin_port = htons(ServerPort);
    assert(bind(lfd, (struct sockaddr*)&saddr, sizeof(saddr)) != -1);

    // listen
    assert(listen(lfd, 5) != -1);

    // epoll
    assert(epoller->AddFd(lfd, ListenEvent | EPOLLIN));

    // NonBlocking
    SetNonBlocking(lfd);
}

void Server::CloseConn(int fd) {
    if (!Clients[fd].isOnline) Clients[fd].name = "NameUnknown";
    string msg = Clients[fd].name + " Left. Online Num: " + to_string(--CurUserCnt);

    Clients.erase(fd);
    epoller->DelFd(fd);
    
    MsgQ.push(SockMsg(-1, msg));
    pool->addTask(bind(&Server::HandleMsg, this, BROADCAST));
}

int Server::SetNonBlocking(int fd) {
    assert(fd > 0);
    return fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0) | O_NONBLOCK);
}
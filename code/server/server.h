#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <memory>
#include <sys/socket.h>

#include <queue>
#include <string>
#include <unordered_map>

#include "epoller.h"
#include "../pool/threadpool.h"
#include "../log/logger.h"
#include "../sql/sql.h"

struct Client {
    int sockfd;
    std::string name;
    bool isOnline;
    Client(): sockfd(0), name(""), isOnline(false) {}
};

struct SockMsg {
    int sockfd;
    string msg;
    SockMsg(int sock, string m): sockfd(sock), msg(m) {}
    SockMsg(): sockfd(0), msg("") {}
};

class Server {
public:
    Server();
    ~Server();

    enum MsgMode {SELF, BROADCAST};

    void HandleMsg(MsgMode mode);
    void HandleRecv(Client* client);

    void HandleConnect();
    void CloseConn(int fd);

    void HandleLogin(int fd);
    void HandleRegister(int fd);
    void HandleStart(int fd);

    void SockInit();
    void Start();

private:
    int SetNonBlocking(int fd);
    const static int BufferLen = 1024;
    const static int MaxClientNum = 32;

    static int CurUserCnt;

    std::unordered_map<int, Client> Clients;

    int lfd;
    int ServerPort;
    std::string ServerIP;
    
    std::unique_ptr<Epoller> epoller;
    std::unique_ptr<ThreadPool> pool;
    std::unique_ptr<Logger> logger;
    uint32_t ListenEvent;
    uint32_t ConnEvent;

    queue<SockMsg> MsgQ;
    std::unique_ptr<User_manager> manager;
};
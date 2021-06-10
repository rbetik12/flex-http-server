#include <vector>
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <thread>
#include <condition_variable>
#include "http/HTTPRequestHandler.h"
#include "threading/ThreadPool.h"

class Server {
public:
    static Server* GetInstance();
    static Server* Create(const char* ip, short port);
    bool IsRunning();
    ~Server();

    void Run();
    void Shutdown();
private:
    static void HandleSIGTERM(int signal);
    static void HandleSIGHUP(int signal);
    char* SockAddrToStr(const struct sockaddr *sa, char *s, size_t maxLen);

    Server(const char* ip, short port);
    void AcceptRequestThread();
private:
    static Server* instance;

    int serverFd;
    bool isRunning;
    struct sockaddr_in address;
    HTTPRequestHandler requestParser;
    std::condition_variable mainThreadCv;
    std::mutex mainThreadMutex;
    std::unique_ptr<std::thread> acceptThreadPtr;
    std::unique_ptr<ThreadPool> threadPool;
};
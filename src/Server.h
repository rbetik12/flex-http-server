#include <vector>
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <thread>
#include "HTTPRequestParser.h"

class Server {
public:
    static Server* GetInstance();
    static Server* Create(short port);

    ~Server();

    void Run();
    void Shutdown();
private:
    static void HandleSIGTERM(int signal);
    static void HandleSIGHUP(int signal);
    char* SockAddrToStr(const struct sockaddr *sa, char *s, size_t maxLen);

    Server(short port);
private:
    static Server* instance;

    int serverFd;
    bool isRunning;
    struct sockaddr_in address;
    HTTPRequestParser requestParser;
};
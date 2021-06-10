#include "Server.h"
#include <iostream>
#include <memory>
#include <signal.h>
#include <fcntl.h>
#include <deque>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <Log.h>

#define IP_ADDRESS "192.168.1.62"

int main(int argc, char *argv[]) {
    short port;
    std::unique_ptr<Server> server;

    if (argc < 2) {
        fputs("server <port>", stderr);
        exit(EXIT_FAILURE);
    }

    if (!(port = atoi(argv[1]))) {
        fputs("Port must be a number between 0 and 65536", stderr);
        exit(EXIT_FAILURE);
    }

    server.reset(Server::Create(port));
    server->Run();
    return 0;
}

Server::Server(short port) : isRunning(false) {
    int opt;

    if ((serverFd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Can't create server socket");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                   &opt, sizeof(opt))) {
        perror("Can't set socket op");
        close(serverFd);
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    inet_aton(IP_ADDRESS, &address.sin_addr);
    address.sin_port = htons(port);

    if (bind(serverFd, (struct sockaddr *) &address,
             sizeof(address)) < 0) {
        perror("Bind failed");
        close(serverFd);
        exit(EXIT_FAILURE);
    }

    if (listen(serverFd, 1024) < 0) {
        perror("Listen failed");
        close(serverFd);
        exit(EXIT_FAILURE);
    }

    Log::Init();

    INFO("Server listening at {0}:{1}", IP_ADDRESS, port);

    const size_t threadsAmount = std::thread::hardware_concurrency();
    auto* threadPoolPtr = new ThreadPool(threadsAmount);
    threadPool.reset(threadPoolPtr);
    INFO("Successfully initialized {0} worker thread", threadsAmount);
}

Server::~Server() {
    close(serverFd);
}

char *Server::SockAddrToStr(const struct sockaddr *sa, char *s, size_t maxLen) {
    switch (sa->sa_family) {
        case AF_INET:
            inet_ntop(AF_INET, &(((struct sockaddr_in *) sa)->sin_addr),
                      s, maxLen);
            break;

        case AF_INET6:
            inet_ntop(AF_INET6, &(((struct sockaddr_in6 *) sa)->sin6_addr),
                      s, maxLen);
            break;

        default:
            strncpy(s, "Unknown AF", maxLen);
            return NULL;
    }

    return s;
}

void Server::Run() {
    acceptThreadPtr = std::make_unique<std::thread>(&Server::AcceptRequestThread, this);
    acceptThreadPtr->detach();
    isRunning = true;

    signal(SIGTERM, Server::HandleSIGTERM);
    signal(SIGHUP, Server::HandleSIGHUP);

    while(isRunning) {
        std::unique_lock<std::mutex> lock(mainThreadMutex);
        mainThreadCv.wait(lock, [] {return !Server::GetInstance()->IsRunning();});
    }

    acceptThreadPtr.reset();
    close(serverFd);
}

void Server::HandleSIGTERM(int signal) {
    Server::GetInstance()->Shutdown();
}

void Server::HandleSIGHUP(int signal) {
    Server::GetInstance()->Shutdown();
}

void Server::AcceptRequestThread() {
    int clientSocket;
    int addrLen = sizeof(address);
    struct sockaddr sockaddr;
    char *newAddress = new char[sizeof(sockaddr.sa_data)];
    INFO("Started accepting thread");
    while (isRunning) {
        if ((clientSocket = accept(serverFd, &sockaddr, (socklen_t *) &addrLen)) < 0) {
            perror("Can't accept new request");
        } else {
            newAddress = SockAddrToStr(&sockaddr, newAddress, sizeof(sockaddr.sa_data));
            TRACE("New request from {0}", newAddress);
            threadPool->Enqueue([=] {
                HTTPSocket socket{};
                socket.socketFd = clientSocket;
                requestParser.Handle(socket);
            });
        }
    }
    INFO("Stopped accepting thread");
    delete[] newAddress;
}

bool Server::IsRunning() {
    return isRunning;
}

Server *Server::GetInstance() {
    return instance;
}

Server *Server::Create(short port) {
    if (instance == nullptr) {
        instance = new Server(port);
    }
    return instance;
}

Server *Server::instance = nullptr;

void Server::Shutdown() {
    isRunning = false;
    mainThreadCv.notify_one();
    INFO("Shutting down server!");
}

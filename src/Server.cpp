#include "Server.h"
#include <iostream>
#include <memory>
#include <signal.h>
#include <fcntl.h>
#include <deque>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <Log.h>

int main(int argc, char *argv[]) {
    short port;
    std::unique_ptr<Server> server;

    if (argc < 3) {
        fputs("server <ip address> <port>", stderr);
        exit(EXIT_FAILURE);
    }

    if (!(port = atoi(argv[2]))) {
        fputs("Port must be a number between 0 and 65536", stderr);
        exit(EXIT_FAILURE);
    }

    server.reset(Server::Create(argv[1], port));
    server->Run();
    return 0;
}

Server::Server(const char* ip, short port) : isRunning(false) {
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
    if (!inet_aton(ip, &address.sin_addr)) {
        perror("Incorrect ipv4 address provided");
        close(serverFd);
        exit(EXIT_FAILURE);
    }
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

    INFO("Server listening at {0}:{1}", ip, port);

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

Server *Server::Create(const char* ip, short port) {
    if (instance == nullptr) {
        instance = new Server(ip, port);
    }
    return instance;
}

Server *Server::instance = nullptr;

void Server::Shutdown() {
    isRunning = false;
    mainThreadCv.notify_one();
    INFO("Shutting down server!");
}

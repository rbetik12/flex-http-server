#pragma once

#include <string>
#include <vector>
#include <memory>
#include "HTTPRequest.h"
#include "HTTPSocket.h"
#include "../socket/ISocketIO.h"

class HTTPRequestHandler {
public:
    HTTPRequestHandler();
    ~HTTPRequestHandler();

    void Handle(HTTPSocket socket);

private:
    void Respond(const HTTPRequest& request, HTTPSocket socket);
    void Parse(std::string& rawRequest, HTTPSocket socket);
    std::vector<std::string> Split(std::string str, std::string delim);
    ISocketIO* socketIo = nullptr;
};

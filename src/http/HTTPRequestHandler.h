#pragma once

#include <string>
#include <vector>
#include "HTTPRequest.h"

class HTTPRequestHandler {
public:
    HTTPRequestHandler();
    ~HTTPRequestHandler();

    void Handle(int socket);

private:
    void Respond(const HTTPRequest& request, int socket);
    void Parse(std::string& rawRequest, int socket);
    std::vector<std::string> Split(std::string str, std::string delim);
};

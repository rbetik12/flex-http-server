#pragma once

#include <string>
#include <vector>
#include "http/HTTPRequest.h"

class HTTPRequestParser {
public:
    HTTPRequestParser();
    ~HTTPRequestParser();

    void Handle(int socket);

private:
    void Respond(const HTTPRequest& request, int socket);
    void Parse(std::string& rawRequest, int socket);
    std::vector<std::string> Split(std::string str, std::string delim);
};

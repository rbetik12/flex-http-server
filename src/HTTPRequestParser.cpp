#include "HTTPRequestParser.h"
#include "http/HTTPRequest.h"
#include <sstream>
#include <iostream>
#include <poll.h>
#include <unistd.h>

#define REQUEST_BUFFER_SIZE 4096
#define RESPONSE_BUFFER_SIZE 8192

HTTPRequestParser::HTTPRequestParser() {

}

HTTPRequestParser::~HTTPRequestParser() {

}

void HTTPRequestParser::Handle(int socket) {
    struct pollfd fd;
    int readAmount;
    char *rawRequest = new char[REQUEST_BUFFER_SIZE];
    fd.fd = socket;
    fd.events = POLLIN;

    int ret = poll(&fd, 1, 100);
    if (ret == -1) {
        throw std::exception();
    } else if (ret == 0) {
        std::cout << "Request timed out!" << std::endl;
    } else {
        readAmount = read(socket, rawRequest, REQUEST_BUFFER_SIZE);
        fd.revents = 0;

        if (readAmount > 0) {
            std::string rawRequestStr(rawRequest);
            Parse(rawRequestStr, socket);
        }
    }

    delete[] rawRequest;
}

void HTTPRequestParser::Respond(const HTTPRequest& request, int socket) {
    char *rawResponse = new char[RESPONSE_BUFFER_SIZE];

    snprintf(rawResponse, RESPONSE_BUFFER_SIZE, "%s 200 OK\nContent-Type: text/plain\nContent-Length: 12\n"
                                                "\nHello world!", request.httpVersion.c_str());
    write(socket, rawResponse, RESPONSE_BUFFER_SIZE);
    close(socket);
    delete[] rawResponse;
}

std::vector<std::string> HTTPRequestParser::Split(std::string str, std::string delim) {
    std::vector<std::string> stringArray;

    size_t pos = 0;
    std::string token;
    while ((pos = str.find(delim)) != std::string::npos) {
        token = str.substr(0, pos);
        stringArray.push_back(token);
        str.erase(0, pos + delim.length());
    }

    if (str[str.size() - 1] == '\n'
        || str[str.size() - 1] == '\r') {
        str[str.size() - 1] = '\0';
    }
    stringArray.push_back(str);

    return stringArray;
}

void HTTPRequestParser::Parse(std::string &rawRequest, int socket) {
    std::vector<std::string> httpHeaderFields;
    std::string httpHeader;
    HTTPRequest request;
    std::stringstream ss(rawRequest);

    std::getline(ss, httpHeader);
    httpHeaderFields = Split(httpHeader, " ");

    request.method = HTTPMethod::GET;
    if (httpHeaderFields[0] == "POST") {
        request.method = HTTPMethod::POST;
    }
    request.requestURI = httpHeaderFields[1];
    request.httpVersion = httpHeaderFields[2];

    Respond(request, socket);
}

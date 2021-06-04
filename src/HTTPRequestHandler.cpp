#include "HTTPRequestHandler.h"
#include "http/HTTPRequest.h"
#include <sstream>
#include <iostream>
#include <poll.h>
#include <unistd.h>
#include <cstring>
#include <regex>

#define REQUEST_BUFFER_SIZE 4096
#define RESPONSE_BUFFER_SIZE 8192
#define FILE_BUFFER_SIZE 4096

HTTPRequestHandler::HTTPRequestHandler() {

}

HTTPRequestHandler::~HTTPRequestHandler() {

}

void HTTPRequestHandler::Handle(int socket) {
    struct pollfd fd;
    int readAmount;
    char *rawRequest = new char[REQUEST_BUFFER_SIZE];
    fd.fd = socket;
    fd.events = POLLIN;

    int ret = poll(&fd, 1, 100);
    if (ret == -1) {
        throw std::exception();
    } else if (ret == 0) {
        close(socket);
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

void HTTPRequestHandler::Respond(const HTTPRequest &request, int socket) {
    char *rawResponse = new char[RESPONSE_BUFFER_SIZE];
    char *fileBuffer = nullptr;
    size_t fileReadAmount = 0;
    FILE *fileToSend;
    std::regex htmlRegex(".html|.htm", std::regex_constants::ECMAScript);

    fileToSend = fopen(("." + request.requestURI).c_str(), "rb");
    if (fileToSend == nullptr) {
        std::string errorMessage = "File " + request.requestURI + " wasn't found!\n";
        snprintf(rawResponse, RESPONSE_BUFFER_SIZE,
                 "%s 404 NOTFOUND\nContent-Type: text/plain\nContent-Length: %lu\n\n%s",
                 request.httpVersion.c_str(), errorMessage.size(), errorMessage.c_str());
        write(socket, rawResponse, strlen(rawResponse));
    } else {
        fileBuffer = new char[FILE_BUFFER_SIZE];
        fileReadAmount = fread(fileBuffer, sizeof(char), FILE_BUFFER_SIZE, fileToSend);
        if (std::regex_search(request.requestURI, htmlRegex)) {
            snprintf(rawResponse, RESPONSE_BUFFER_SIZE, "%s 200 OK\nContent-Type: text/html\nContent-Length: %lu\n"
                                                        "\n%s", request.httpVersion.c_str(), fileReadAmount,
                     fileBuffer);
            write(socket, rawResponse, strlen(rawResponse));
        }

        delete[] fileBuffer;
    }
    close(socket);
    delete[] rawResponse;
}

std::vector<std::string> HTTPRequestHandler::Split(std::string str, std::string delim) {
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

void HTTPRequestHandler::Parse(std::string &rawRequest, int socket) {
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

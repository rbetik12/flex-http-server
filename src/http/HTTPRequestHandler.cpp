#include <iostream>
#include <poll.h>
#include <cstring>
#include <regex>
#include "HTTPRequestHandler.h"
#include "HTTPRequest.h"
#include "../socket/ISocketIOFactory.h"
#include <Log.h>

#define REQUEST_BUFFER_SIZE 4096
#define RESPONSE_BUFFER_SIZE 4096
#define FILE_BUFFER_SIZE 4096

HTTPRequestHandler::HTTPRequestHandler() {
    socketIo = ISocketIOFactory::GetInstance();
}

HTTPRequestHandler::~HTTPRequestHandler() {
    delete socketIo;
}

void HTTPRequestHandler::Handle(HTTPSocket socket) {
    struct pollfd fd;
    int readAmount;
    char *rawRequest = new char[REQUEST_BUFFER_SIZE];
    fd.fd = socket.socketFd;
    fd.events = POLLIN;

    int ret = poll(&fd, 1, 100);
    if (ret == -1) {
        throw std::exception();
    } else if (ret == 0) {
        socketIo->Close(socket);
    } else {
        readAmount = socketIo->Read(socket, rawRequest, REQUEST_BUFFER_SIZE);
        fd.revents = 0;

        if (readAmount > 0) {
            std::string rawRequestStr(rawRequest);
            Parse(rawRequestStr, socket);
        }
    }

    delete[] rawRequest;
}

void HTTPRequestHandler::Respond(const HTTPRequest &request, HTTPSocket socket) {
    std::vector<char> rawHTTPResponseHeader;
    std::vector<char> fileBuffer;
    std::array<char, FILE_BUFFER_SIZE> readFileBuffer{};
    size_t fileReadAmount = 0;
    FILE *fileToSend;
    std::regex htmlRegex(".html|.htm", std::regex_constants::ECMAScript);
    std::regex faviconRegex("favicon.ico", std::regex_constants::ECMAScript);

    fileToSend = fopen(("." + request.requestURI).c_str(), "rb");
    rawHTTPResponseHeader.reserve(RESPONSE_BUFFER_SIZE);
    if (fileToSend == nullptr) {
        std::string errorMessage = "File " + request.requestURI + " wasn't found!\n";
        snprintf(rawHTTPResponseHeader.data(), RESPONSE_BUFFER_SIZE,
                 "%s 404 NOTFOUND\nContent-Type: text/plain\nContent-Length: %lu\n\n%s",
                 request.httpVersion.c_str(), errorMessage.size(), errorMessage.c_str());
        socketIo->Write(socket, rawHTTPResponseHeader.data(), strlen(rawHTTPResponseHeader.data()));
    } else {
        while ((fileReadAmount = fread(readFileBuffer.data(), sizeof(char), FILE_BUFFER_SIZE, fileToSend)) > 0) {
            fileBuffer.insert(fileBuffer.end(), readFileBuffer.data(), readFileBuffer.data() + fileReadAmount);
        }

        if (std::regex_search(request.requestURI, htmlRegex)) {
            snprintf(rawHTTPResponseHeader.data(), rawHTTPResponseHeader.capacity(),
                     "%s 200 OK\nContent-Type: text/html\nContent-Length: %lu\n\n",
                     request.httpVersion.c_str(), fileBuffer.size());
            socketIo->Write(socket, rawHTTPResponseHeader.data(), strlen(rawHTTPResponseHeader.data()));
            socketIo->Write(socket, fileBuffer.data(), fileBuffer.size());
        }
        else if (std::regex_search(request.requestURI, faviconRegex)) {
            snprintf(rawHTTPResponseHeader.data(), rawHTTPResponseHeader.capacity(),
                     "%s 200 OK\nContent-Type: image/x-icon\nContent-Length: %lu\n\n",
                     request.httpVersion.c_str(), fileBuffer.size());
            socketIo->Write(socket, rawHTTPResponseHeader.data(), strlen(rawHTTPResponseHeader.data()));
            socketIo->Write(socket, fileBuffer.data(), fileBuffer.size());
        }
    }
    socketIo->Close(socket);
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

void HTTPRequestHandler::Parse(std::string &rawRequest, HTTPSocket socket) {
    std::vector<std::string> httpHeaderFields;
    std::string httpHeader;
    HTTPRequest request;
    std::stringstream ss(rawRequest);

    std::getline(ss, httpHeader);
    httpHeaderFields = Split(httpHeader, " ");

    request.method = HTTPMethod::GET;
    if (httpHeaderFields[0] == "POST") {
        request.method = HTTPMethod::POST;
        WARN("POST method request supported is not implemented yet!");
    }
    request.requestURI = httpHeaderFields[1];
    request.httpVersion = httpHeaderFields[2];

    Respond(request, socket);
}

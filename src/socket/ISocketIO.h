#pragma once

#include "../http/HTTPSocket.h"

class ISocketIO {
public:
    virtual ssize_t Write(HTTPSocket& socket, void* data, size_t size) = 0;
    virtual ssize_t Read(HTTPSocket& socket, void* buffer, size_t size) = 0;
    virtual void Close(HTTPSocket& socket) = 0;
};
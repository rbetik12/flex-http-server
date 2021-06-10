#include <cstddef>
#include "../http/HTTPSocket.h"
#include "ISocketIO.h"

class LinuxSocketIO: public ISocketIO {
public:
    ssize_t Write(HTTPSocket &socket, void *data, size_t size) override;

    ssize_t Read(HTTPSocket &socket, void *buffer, size_t size) override;

    void Close(HTTPSocket &socket) override;
};


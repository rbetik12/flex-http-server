#include <unistd.h>
#include "LinuxSocketIO.h"

ssize_t LinuxSocketIO::Write(HTTPSocket &socket, void *data, size_t size) {
    return write(socket.socketFd, data, size);
}

ssize_t LinuxSocketIO::Read(HTTPSocket &socket, void *buffer, size_t size) {
    return read(socket.socketFd, buffer, size);
}

void LinuxSocketIO::Close(HTTPSocket &socket) {
    close(socket.socketFd);
}

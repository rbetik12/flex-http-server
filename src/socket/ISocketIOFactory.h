#pragma once
#include "LinuxSocketIO.h"

class ISocketIOFactory {
public:
    static ISocketIO* GetInstance() {
#ifdef UNIX
        if (instance == nullptr) {
            instance = new LinuxSocketIO();
        }
#endif
        return instance;
    }

private:
    static ISocketIO* instance;
};

ISocketIO* ISocketIOFactory::instance = nullptr;
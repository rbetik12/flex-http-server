cmake_minimum_required(VERSION 3.17)
project(http_server)

set(CMAKE_CXX_STANDARD 17)

if(UNIX)
    add_compile_definitions(UNIX)
endif()

add_subdirectory(lib/spdlog)

include_directories(lib/spdlog/include)
include_directories(src/log)

add_executable(http_server src/Server.h src/Server.cpp src/http/HTTPRequestHandler.cpp src/http/HTTPRequestHandler.h src/http/HTTPRequest.h src/threading/ThreadPool.cpp src/threading/ThreadPool.h src/log/Log.cpp src/log/Log.h src/http/HTTPSocket.h src/socket/ISocketIO.h src/socket/LinuxSocketIO.cpp src/socket/LinuxSocketIO.h src/socket/ISocketIOFactory.h)
target_link_libraries(http_server pthread)
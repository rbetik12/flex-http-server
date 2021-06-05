cmake_minimum_required(VERSION 3.17)
project(http_server)

set(CMAKE_CXX_STANDARD 17)

add_executable(http_server src/Server.h src/Server.cpp src/http/HTTPRequestHandler.cpp src/http/HTTPRequestHandler.h src/http/HTTPRequest.h src/threading/ThreadPool.cpp src/threading/ThreadPool.h)
target_link_libraries(http_server pthread)
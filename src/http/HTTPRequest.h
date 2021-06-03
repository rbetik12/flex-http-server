#pragma once

enum class HTTPMethod {
    GET,
    POST,
    PUT,
    DELETE
};

struct HTTPRequest {
    HTTPMethod method;
    std::string requestURI;
    std::string httpVersion;
};
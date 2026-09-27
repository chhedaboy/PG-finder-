// HttpServer.h - a very small HTTP server (plain sockets, no external library).
// It handles one request at a time, which is enough for a college project.
#pragma once
#include <functional>
#include <map>
#include <string>

struct HttpRequest {
    std::string method;                          // "GET" or "POST"
    std::string path;                            // e.g. "/api/search"
    std::map<std::string, std::string> params;   // query string + form body, already decoded
};

struct HttpResponse {
    int status = 200;
    std::string contentType = "application/json";
    std::string body;
};

class HttpServer {
public:
    typedef std::function<HttpResponse(const HttpRequest&)> Handler;

    HttpServer(int port, Handler handler);
    bool run();   // runs forever (returns false if the port could not be opened)

private:
    int port;
    Handler handler;
};

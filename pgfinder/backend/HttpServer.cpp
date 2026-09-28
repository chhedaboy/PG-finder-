#include "HttpServer.h"
#include <cstdio>
#include <cstring>
#include <iostream>
#include "Util.h"

#ifdef _WIN32
  #include <winsock2.h>
  #include <ws2tcpip.h>
  #ifdef _MSC_VER
    #pragma comment(lib, "ws2_32.lib")
  #endif
  typedef SOCKET sock_t;
  #define CLOSE_SOCKET closesocket
  #define BAD_SOCKET INVALID_SOCKET
#else
  #include <arpa/inet.h>
  #include <csignal>
  #include <netinet/in.h>
  #include <sys/socket.h>
  #include <sys/time.h>
  #include <unistd.h>
  typedef int sock_t;
  #define CLOSE_SOCKET close
  #define BAD_SOCKET (-1)
#endif

// ---- helpers ---------------------------------------------------------------

static int hexValue(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

static std::string urlDecode(const std::string& s) {
    std::string out;
    for (size_t i = 0; i < s.size(); i++) {
        if (s[i] == '+') out += ' ';
        else if (s[i] == '%' && i + 2 < s.size() &&
                 hexValue(s[i + 1]) >= 0 &&
                 hexValue(s[i + 2]) >= 0) {
            out += (char)(hexValue(s[i + 1]) * 16 +
                          hexValue(s[i + 2]));
            i += 2;
        } else {
            out += s[i];
        }
    }
    return out;
}

// "a=1&b=hello+world" -> {a:"1", b:"hello world"}
static void parseParams(
    const std::string& text,
    std::map<std::string, std::string>& params
) {
    for (const std::string& pair : splitStr(text, '&')) {
        if (pair.empty()) continue;

        size_t eq = pair.find('=');

        if (eq == std::string::npos)
            params[urlDecode(pair)] = "";
        else
            params[urlDecode(pair.substr(0, eq))] =
                urlDecode(pair.substr(eq + 1));
    }
}

static const char* statusText(int code) {
    switch (code) {
        case 200: return "OK";
        case 400: return "Bad Request";
        case 403: return "Forbidden";
        case 404: return "Not Found";
        default:  return "Internal Server Error";
    }
}

// Reads a full request (headers + body) from the socket.
static bool readRequest(sock_t client, HttpRequest& req) {
    std::string data;
    char buf[4096];
    size_t headerEnd = std::string::npos;
    size_t contentLength = 0;

    while (true) {
        if (headerEnd == std::string::npos) {
            headerEnd = data.find("\r\n\r\n");

            if (headerEnd != std::string::npos) {
                // Find Content-Length in the headers
                std::string head =
                    toLowerStr(data.substr(0, headerEnd));

                size_t p = head.find("content-length:");

                if (p != std::string::npos)
                    contentLength =
                        (size_t)std::atol(head.c_str() + p + 15);

                headerEnd += 4;
            }
        }

        if (headerEnd != std::string::npos &&
            data.size() >= headerEnd + contentLength)
            break;

        if (data.size() > 1000000)
            return false;

        int n = (int)recv(client, buf, sizeof(buf), 0);

        if (n <= 0)
            return false;

        data.append(buf, n);
    }

    // request line: METHOD TARGET HTTP/1.1
    size_t lineEnd = data.find("\r\n");

    std::vector<std::string> parts =
        splitStr(data.substr(0, lineEnd), ' ');

    if (parts.size() < 2)
        return false;

    req.method = parts[0];

    std::string target = parts[1];

    size_t q = target.find('?');

    if (q != std::string::npos) {
        parseParams(target.substr(q + 1), req.params);
        target = target.substr(0, q);
    }

    req.path = urlDecode(target);

    if (req.method == "POST")
        parseParams(
            data.substr(headerEnd, contentLength),
            req.params
        );

    return true;
}

static void sendResponse(
    sock_t client,
    const HttpResponse& res
) {
    std::string out =
        std::string("HTTP/1.1 ") +
        std::to_string(res.status) + " " +
        statusText(res.status) + "\r\n" +

        "Content-Type: " + res.contentType + "\r\n" +

        "Content-Length: " +
        std::to_string(res.body.size()) + "\r\n" +

        // Allow the Vercel frontend to call this backend
        "Access-Control-Allow-Origin: *\r\n" +

        "Connection: close\r\n\r\n" +

        res.body;

    size_t sent = 0;

    while (sent < out.size()) {
        int n = (int)send(
            client,
            out.data() + sent,
            (int)(out.size() - sent),
            0
        );

        if (n <= 0)
            break;

        sent += n;
    }
}

// ---- server ----------------------------------------------------------------

HttpServer::HttpServer(int port, Handler handler)
    : port(port), handler(handler) {}

bool HttpServer::run() {

#ifdef _WIN32

    WSADATA wsa;

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        return false;

#else

    std::signal(SIGPIPE, SIG_IGN);

#endif

    sock_t server =
        socket(AF_INET, SOCK_STREAM, 0);

    if (server == BAD_SOCKET)
        return false;

#ifndef _WIN32

    int yes = 1;

    setsockopt(
        server,
        SOL_SOCKET,
        SO_REUSEADDR,
        (const char*)&yes,
        sizeof(yes)
    );

#endif

    sockaddr_in addr;

    std::memset(
        &addr,
        0,
        sizeof(addr)
    );

    addr.sin_family = AF_INET;

    // IMPORTANT:
    // INADDR_ANY allows Render / external clients
    // to connect to the server.
    addr.sin_addr.s_addr =
        htonl(INADDR_ANY);

    addr.sin_port =
        htons((unsigned short)port);

    if (bind(
            server,
            (sockaddr*)&addr,
            sizeof(addr)
        ) != 0)
        return false;

    if (listen(server, 16) != 0)
        return false;

    while (true) {

        sock_t client =
            accept(server, nullptr, nullptr);

        if (client == BAD_SOCKET)
            continue;

        // Browsers sometimes open idle connections;
        // don't wait for them for too long.

#ifdef _WIN32

        DWORD timeoutMs = 400;

        setsockopt(
            client,
            SOL_SOCKET,
            SO_RCVTIMEO,
            (const char*)&timeoutMs,
            sizeof(timeoutMs)
        );

#else

        timeval tv;

        tv.tv_sec = 0;
        tv.tv_usec = 400000;

        setsockopt(
            client,
            SOL_SOCKET,
            SO_RCVTIMEO,
            (const char*)&tv,
            sizeof(tv)
        );

#endif

        HttpRequest req;

        if (readRequest(client, req))
            sendResponse(
                client,
                handler(req)
            );

        CLOSE_SOCKET(client);
    }

    return true;
}
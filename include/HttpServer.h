#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <string>
#include <map>
#include <functional>
#include <winsock2.h>
#include <windows.h>
#include "CodeVault.h"
#include "CodeSentry.h"
#include "AlgoPro.h"
#include "ScoringEngine.h"
#include "Database.h"

// HTTP Request representation
struct HttpRequest {
    std::string method;
    std::string path;
    std::string queryString;
    std::map<std::string, std::string> headers;
    std::string body;
};

// HTTP Response representation
struct HttpResponse {
    int statusCode;
    std::string statusText;
    std::string contentType;
    std::string body;

    HttpResponse() : statusCode(200), statusText("OK"), contentType("application/json; charset=utf-8"), body("") {}
};

// Native C++ HTTP Web Server
class HttpServer {
private:
    int port;
    std::string publicDir;
    SOCKET listenSocket;
    bool isRunning;

    // Core System Subsystems
    CodeVault codeVault;
    AlgoPro algoPro;
    ScoringEngine scoringEngine;
    Database database;

    void handleClient(SOCKET clientSocket);
    static DWORD WINAPI clientThreadRouter(LPVOID lpParam);

    HttpRequest parseRequest(const std::string& raw);
    std::string buildResponse(const HttpResponse& resp);

    // REST API Handlers
    HttpResponse handleGetStatus();
    HttpResponse handleGetLeaderboard();
    HttpResponse handleGetHistory();
    HttpResponse handlePostEvaluate(const std::string& body);
    HttpResponse handlePostProfile(const std::string& body);
    HttpResponse handlePostJudge(const std::string& body);
    HttpResponse handlePostCommit(const std::string& body);
    HttpResponse handleStaticFile(const std::string& path);

    std::string getMimeType(const std::string& filePath);

public:
    HttpServer(int port, const std::string& publicDir);
    ~HttpServer();

    bool start();
    void stop();
};

#endif // HTTP_SERVER_H

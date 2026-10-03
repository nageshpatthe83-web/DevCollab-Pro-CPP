#include "../include/HttpServer.h"
#include "../include/ConfigManager.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

// JSON string escaping helper
static std::string escapeJson(const std::string& s) {
    std::string out;
    for (char c : s) {
        if (c == '"') out += "\\\"";
        else if (c == '\\') out += "\\\\";
        else if (c == '\b') out += "\\b";
        else if (c == '\f') out += "\\f";
        else if (c == '\n') out += "\\n";
        else if (c == '\r') out += "\\r";
        else if (c == '\t') out += "\\t";
        else out += c;
    }
    return out;
}

// Simple JSON extraction helper
static std::string extractJsonString(const std::string& json, const std::string& key) {
    std::string search = "\"" + key + "\":";
    size_t pos = json.find(search);
    if (pos == std::string::npos) return "";

    pos += search.length();
    while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t' || json[pos] == '\r' || json[pos] == '\n')) {
        pos++;
    }

    if (pos < json.size() && json[pos] == '"') {
        pos++;
        size_t end = json.find('"', pos);
        if (end != std::string::npos) {
            std::string res = json.substr(pos, end - pos);
            // unescape basic \n and \"
            std::string unescaped;
            for (size_t k = 0; k < res.size(); ++k) {
                if (res[k] == '\\' && k + 1 < res.size()) {
                    if (res[k+1] == 'n') { unescaped += '\n'; k++; }
                    else if (res[k+1] == 'r') { unescaped += '\r'; k++; }
                    else if (res[k+1] == 't') { unescaped += '\t'; k++; }
                    else if (res[k+1] == '"') { unescaped += '"'; k++; }
                    else if (res[k+1] == '\\') { unescaped += '\\'; k++; }
                    else { unescaped += res[k]; }
                } else {
                    unescaped += res[k];
                }
            }
            return unescaped;
        }
    }
    return "";
}

static double extractJsonDouble(const std::string& json, const std::string& key, double defaultVal = 0.0) {
    std::string search = "\"" + key + "\":";
    size_t pos = json.find(search);
    if (pos == std::string::npos) return defaultVal;

    pos += search.length();
    while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t')) pos++;

    try {
        return std::stod(json.substr(pos));
    } catch (...) {
        return defaultVal;
    }
}

HttpServer::HttpServer(int p, const std::string& pub) 
    : port(p), publicDir(pub), listenSocket(INVALID_SOCKET), isRunning(false) {
    // Register database as observer of scoring engine (Observer Pattern)
    // Note: Database is a direct member, we can also link via shared_ptr
}

HttpServer::~HttpServer() {
    stop();
}

std::string HttpServer::getMimeType(const std::string& filePath) {
    if (filePath.find(".html") != std::string::npos) return "text/html; charset=utf-8";
    if (filePath.find(".css") != std::string::npos) return "text/css; charset=utf-8";
    if (filePath.find(".js") != std::string::npos) return "application/javascript; charset=utf-8";
    if (filePath.find(".json") != std::string::npos) return "application/json; charset=utf-8";
    if (filePath.find(".svg") != std::string::npos) return "image/svg+xml";
    if (filePath.find(".png") != std::string::npos) return "image/png";
    return "text/plain; charset=utf-8";
}

HttpRequest HttpServer::parseRequest(const std::string& raw) {
    HttpRequest req;
    req.method = "GET";
    req.path = "/";

    std::stringstream ss(raw);
    std::string line;
    if (std::getline(ss, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        std::stringstream lineSs(line);
        lineSs >> req.method >> req.path;

        size_t qpos = req.path.find('?');
        if (qpos != std::string::npos) {
            req.queryString = req.path.substr(qpos + 1);
            req.path = req.path.substr(0, qpos);
        }
    }

    // Parse Headers
    while (std::getline(ss, line) && line != "\r" && !line.empty()) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        size_t colon = line.find(':');
        if (colon != std::string::npos) {
            std::string key = line.substr(0, colon);
            std::string val = line.substr(colon + 1);
            while (!val.empty() && val.front() == ' ') val.erase(0, 1);
            req.headers[key] = val;
        }
    }

    // Remaining is Body
    std::string bodyRest;
    while (std::getline(ss, line)) {
        bodyRest += line + "\n";
    }
    if (!bodyRest.empty() && bodyRest.back() == '\n') bodyRest.pop_back();
    req.body = bodyRest;

    return req;
}

std::string HttpServer::buildResponse(const HttpResponse& resp) {
    std::stringstream ss;
    ss << "HTTP/1.1 " << resp.statusCode << " " << resp.statusText << "\r\n";
    ss << "Content-Type: " << resp.contentType << "\r\n";
    ss << "Content-Length: " << resp.body.size() << "\r\n";
    ss << "Access-Control-Allow-Origin: *\r\n";
    ss << "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n";
    ss << "Access-Control-Allow-Headers: Content-Type\r\n";
    ss << "Connection: close\r\n";
    ss << "\r\n";
    ss << resp.body;
    return ss.str();
}

HttpResponse HttpServer::handleGetStatus() {
    HttpResponse resp;
    long long rawB = 0, deltaB = 0;
    double savings = 0;
    codeVault.getStorageStats(rawB, deltaB, savings);

    std::stringstream ss;
    ss << "{\n";
    ss << "  \"status\": \"online\",\n";
    ss << "  \"platform\": \"" << ConfigManager::getInstance().getPlatformName() << "\",\n";
    ss << "  \"version\": \"" << ConfigManager::getInstance().getVersion() << "\",\n";
    ss << "  \"backend\": \"Native C++14 (OOP Architecture)\",\n";
    ss << "  \"designPatterns\": [\"Strategy\", \"Visitor\", \"Factory\", \"Observer\", \"Singleton\"],\n";
    ss << "  \"totalCommits\": " << codeVault.getHistory().size() << ",\n";
    ss << "  \"rawStorageBytes\": " << rawB << ",\n";
    ss << "  \"deltaStorageBytes\": " << deltaB << ",\n";
    ss << "  \"storageSavingsPercent\": " << savings << "\n";
    ss << "}";
    resp.body = ss.str();
    return resp;
}

HttpResponse HttpServer::handleGetLeaderboard() {
    HttpResponse resp;
    std::vector<LeaderboardEntry> list = database.getRankings();

    std::stringstream ss;
    ss << "[\n";
    for (size_t i = 0; i < list.size(); ++i) {
        const auto& e = list[i];
        ss << "  {\n";
        ss << "    \"rank\": " << e.rank << ",\n";
        ss << "    \"teamId\": \"" << escapeJson(e.teamId) << "\",\n";
        ss << "    \"teamName\": \"" << escapeJson(e.teamName) << "\",\n";
        ss << "    \"projectName\": \"" << escapeJson(e.projectName) << "\",\n";
        ss << "    \"members\": \"" << escapeJson(e.members) << "\",\n";
        ss << "    \"compositeTotal\": " << e.scores.compositeTotal << ",\n";
        ss << "    \"scores\": {\n";
        ss << "      \"functionality\": " << e.scores.functionalityScore << ",\n";
        ss << "      \"codeQuality\": " << e.scores.codeQualityScore << ",\n";
        ss << "      \"performance\": " << e.scores.performanceScore << ",\n";
        ss << "      \"database\": " << e.scores.databaseScore << ",\n";
        ss << "      \"documentation\": " << e.scores.documentationScore << ",\n";
        ss << "      \"security\": " << e.scores.securityScore << ",\n";
        ss << "      \"innovation\": " << e.scores.innovationScore << ",\n";
        ss << "      \"uiUx\": " << e.scores.uiUxScore << ",\n";
        ss << "      \"presentation\": " << e.scores.presentationScore << "\n";
        ss << "    },\n";
        ss << "    \"submissionTime\": \"" << escapeJson(e.submissionTime) << "\",\n";
        ss << "    \"latestCommitHash\": \"" << escapeJson(e.latestCommitHash) << "\"\n";
        ss << "  }" << (i + 1 < list.size() ? "," : "") << "\n";
    }
    ss << "]";
    resp.body = ss.str();
    return resp;
}

HttpResponse HttpServer::handleGetHistory() {
    HttpResponse resp;
    std::vector<CommitRecord> history = codeVault.getHistory();

    std::stringstream ss;
    ss << "[\n";
    for (size_t i = 0; i < history.size(); ++i) {
        const auto& c = history[i];
        ss << "  {\n";
        ss << "    \"commitHash\": \"" << c.commitHash << "\",\n";
        ss << "    \"author\": \"" << escapeJson(c.author) << "\",\n";
        ss << "    \"message\": \"" << escapeJson(c.message) << "\",\n";
        ss << "    \"timestamp\": \"" << c.timestamp << "\",\n";
        ss << "    \"linesAdded\": " << c.linesAdded << ",\n";
        ss << "    \"linesDeleted\": " << c.linesDeleted << ",\n";
        ss << "    \"rawSizeBytes\": " << c.rawSizeBytes << ",\n";
        ss << "    \"deltaSizeBytes\": " << c.deltaSizeBytes << ",\n";
        ss << "    \"storageSavingsPercent\": " << c.storageSavingsPercent << ",\n";
        ss << "    \"deltas\": [\n";
        for (size_t j = 0; j < c.deltas.size(); ++j) {
            const auto& d = c.deltas[j];
            std::string opStr(1, d.operation);
            ss << "      {\"op\": \"" << opStr << "\", \"line\": " << d.lineNumber << ", \"content\": \"" << escapeJson(d.content) << "\"}"
               << (j + 1 < c.deltas.size() ? "," : "") << "\n";
        }
        ss << "    ]\n";
        ss << "  }" << (i + 1 < history.size() ? "," : "") << "\n";
    }
    ss << "]";
    resp.body = ss.str();
    return resp;
}

HttpResponse HttpServer::handlePostEvaluate(const std::string& body) {
    HttpResponse resp;

    std::string code = extractJsonString(body, "code");
    std::string language = extractJsonString(body, "language");
    std::string teamName = extractJsonString(body, "teamName");
    std::string projectName = extractJsonString(body, "projectName");
    std::string author = extractJsonString(body, "author");

    if (code.empty()) {
        code = "#include <iostream>\nusing namespace std;\n\nint main() {\n    cout << \"Hello World\" << endl;\n    return 0;\n}\n";
    }
    if (language.empty()) language = "cpp";
    if (teamName.empty()) teamName = "Team SY06";
    if (projectName.empty()) projectName = "DevCollab Core Module";
    if (author.empty()) author = "Nagesh Patthe";

    // 1. CodeSentry (OOP AST Static Analyzer via Factory Pattern)
    auto analyzer = CodeAnalyzerFactory::createAnalyzer(language);
    QualityMetrics quality = analyzer->analyze(code);

    // 2. AlgoPro (DSA Empirical Profiler)
    AlgorithmProfile profile = algoPro.profileCustomCode(code);

    // 3. CodeVault (DBMS Delta Storage)
    CommitRecord commit = codeVault.commit(author, "Evaluation submission for " + projectName, code);

    // 4. ScoringEngine (Weighted Multi-Dimensional Formula)
    ScoreBreakdown scores = scoringEngine.computeScore(quality, profile, 10, 10, true, 9.0, 8.5, 9.0);

    // 5. Update Database & Max-Heap
    LeaderboardEntry entry;
    entry.teamId = teamName;
    entry.teamName = teamName;
    entry.projectName = projectName;
    entry.members = author;
    entry.submissionTime = commit.timestamp;
    entry.latestCommitHash = commit.commitHash;
    entry.scores = scores;

    database.recordSubmission(entry);

    // Build Response JSON
    std::stringstream ss;
    ss << "{\n";
    ss << "  \"status\": \"success\",\n";
    ss << "  \"teamName\": \"" << escapeJson(teamName) << "\",\n";
    ss << "  \"projectName\": \"" << escapeJson(projectName) << "\",\n";
    ss << "  \"commitHash\": \"" << commit.commitHash << "\",\n";
    ss << "  \"storageSavingsPercent\": " << commit.storageSavingsPercent << ",\n";
    ss << "  \"quality\": {\n";
    ss << "    \"totalLines\": " << quality.totalLines << ",\n";
    ss << "    \"codeLines\": " << quality.codeLines << ",\n";
    ss << "    \"commentLines\": " << quality.commentLines << ",\n";
    ss << "    \"cyclomaticComplexity\": " << quality.cyclomaticComplexity << ",\n";
    ss << "    \"maintainabilityIndex\": " << quality.maintainabilityIndex << ",\n";
    ss << "    \"securityFlawsCount\": " << quality.securityFlawsCount << ",\n";
    ss << "    \"issues\": [\n";
    for (size_t i = 0; i < quality.issues.size(); ++i) {
        const auto& is = quality.issues[i];
        ss << "      {\"type\": \"" << is.type << "\", \"rule\": \"" << is.ruleName << "\", \"line\": " << is.line 
           << ", \"severity\": \"" << is.severity << "\", \"message\": \"" << escapeJson(is.message) << "\"}"
           << (i + 1 < quality.issues.size() ? "," : "") << "\n";
    }
    ss << "    ]\n";
    ss << "  },\n";
    ss << "  \"performance\": {\n";
    ss << "    \"algorithm\": \"" << profile.algorithmName << "\",\n";
    ss << "    \"complexity\": \"" << profile.empiricalComplexity << "\",\n";
    ss << "    \"rSquared\": " << profile.rSquaredFit << ",\n";
    ss << "    \"points\": [\n";
    for (size_t i = 0; i < profile.dataPoints.size(); ++i) {
        const auto& pt = profile.dataPoints[i];
        ss << "      {\"n\": " << pt.inputSize << ", \"timeUs\": " << pt.executionTimeMicroseconds 
           << ", \"comps\": " << pt.comparisons << ", \"swaps\": " << pt.swaps << "}"
           << (i + 1 < profile.dataPoints.size() ? "," : "") << "\n";
    }
    ss << "    ]\n";
    ss << "  },\n";
    ss << "  \"scores\": {\n";
    ss << "    \"compositeTotal\": " << scores.compositeTotal << ",\n";
    ss << "    \"functionality\": " << scores.functionalityScore << ",\n";
    ss << "    \"codeQuality\": " << scores.codeQualityScore << ",\n";
    ss << "    \"performance\": " << scores.performanceScore << ",\n";
    ss << "    \"database\": " << scores.databaseScore << ",\n";
    ss << "    \"documentation\": " << scores.documentationScore << ",\n";
    ss << "    \"security\": " << scores.securityScore << ",\n";
    ss << "    \"innovation\": " << scores.innovationScore << ",\n";
    ss << "    \"uiUx\": " << scores.uiUxScore << ",\n";
    ss << "    \"presentation\": " << scores.presentationScore << "\n";
    ss << "  }\n";
    ss << "}";

    resp.body = ss.str();
    return resp;
}

HttpResponse HttpServer::handlePostProfile(const std::string& body) {
    HttpResponse resp;
    std::string algo = extractJsonString(body, "algorithm");
    if (algo.empty()) algo = "quick_sort";

    AlgorithmProfile profile = algoPro.profileAlgorithm(algo);

    std::stringstream ss;
    ss << "{\n";
    ss << "  \"algorithm\": \"" << profile.algorithmName << "\",\n";
    ss << "  \"empiricalComplexity\": \"" << profile.empiricalComplexity << "\",\n";
    ss << "  \"rSquaredFit\": " << profile.rSquaredFit << ",\n";
    ss << "  \"dataPoints\": [\n";
    for (size_t i = 0; i < profile.dataPoints.size(); ++i) {
        const auto& p = profile.dataPoints[i];
        ss << "    {\"n\": " << p.inputSize << ", \"timeUs\": " << p.executionTimeMicroseconds 
           << ", \"comps\": " << p.comparisons << ", \"swaps\": " << p.swaps << "}"
           << (i + 1 < profile.dataPoints.size() ? "," : "") << "\n";
    }
    ss << "  ]\n";
    ss << "}";

    resp.body = ss.str();
    return resp;
}

HttpResponse HttpServer::handlePostJudge(const std::string& body) {
    HttpResponse resp;
    std::string teamId = extractJsonString(body, "teamId");
    double inno = extractJsonDouble(body, "innovation", 8.5);
    double ui = extractJsonDouble(body, "ui", 8.0);
    double pres = extractJsonDouble(body, "presentation", 8.5);

    bool ok = database.updateJudgeScores(teamId, inno, ui, pres);

    std::stringstream ss;
    ss << "{\"success\": " << (ok ? "true" : "false") << ", \"teamId\": \"" << escapeJson(teamId) << "\"}";
    resp.body = ss.str();
    return resp;
}

HttpResponse HttpServer::handlePostCommit(const std::string& body) {
    HttpResponse resp;
    std::string author = extractJsonString(body, "author");
    std::string message = extractJsonString(body, "message");
    std::string code = extractJsonString(body, "code");

    if (author.empty()) author = "Nagesh";
    if (message.empty()) message = "Code update via DevCollab UI";

    CommitRecord rec = codeVault.commit(author, message, code);

    std::stringstream ss;
    ss << "{\n";
    ss << "  \"commitHash\": \"" << rec.commitHash << "\",\n";
    ss << "  \"linesAdded\": " << rec.linesAdded << ",\n";
    ss << "  \"linesDeleted\": " << rec.linesDeleted << ",\n";
    ss << "  \"rawBytes\": " << rec.rawSizeBytes << ",\n";
    ss << "  \"deltaBytes\": " << rec.deltaSizeBytes << ",\n";
    ss << "  \"storageSavingsPercent\": " << rec.storageSavingsPercent << "\n";
    ss << "}";
    resp.body = ss.str();
    return resp;
}

HttpResponse HttpServer::handleStaticFile(const std::string& rawPath) {
    HttpResponse resp;
    std::string filePath = rawPath;
    if (filePath == "/" || filePath.empty()) {
        filePath = "/index.html";
    }

    std::string fullPath = publicDir + filePath;
    std::ifstream file(fullPath.c_str(), std::ios::binary);
    if (!file.is_open()) {
        resp.statusCode = 404;
        resp.statusText = "Not Found";
        resp.contentType = "text/plain";
        resp.body = "404 Not Found: " + filePath;
        return resp;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    resp.body = buffer.str();
    resp.contentType = getMimeType(filePath);
    return resp;
}

struct ThreadParams {
    HttpServer* server;
    SOCKET client;
};

DWORD WINAPI HttpServer::clientThreadRouter(LPVOID lpParam) {
    ThreadParams* tp = reinterpret_cast<ThreadParams*>(lpParam);
    tp->server->handleClient(tp->client);
    delete tp;
    return 0;
}

void HttpServer::handleClient(SOCKET clientSocket) {
    char buffer[16384];
    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    if (bytesReceived <= 0) {
        closesocket(clientSocket);
        return;
    }
    buffer[bytesReceived] = '\0';

    HttpRequest req = parseRequest(std::string(buffer, bytesReceived));
    HttpResponse resp;

    // Handle OPTIONS for CORS pre-flight
    if (req.method == "OPTIONS") {
        resp.statusCode = 200;
        resp.body = "";
    } else if (req.path == "/api/status" && req.method == "GET") {
        resp = handleGetStatus();
    } else if (req.path == "/api/leaderboard" && req.method == "GET") {
        resp = handleGetLeaderboard();
    } else if (req.path == "/api/version-control/history" && req.method == "GET") {
        resp = handleGetHistory();
    } else if (req.path == "/api/evaluate" && req.method == "POST") {
        resp = handlePostEvaluate(req.body);
    } else if (req.path == "/api/profile" && req.method == "POST") {
        resp = handlePostProfile(req.body);
    } else if (req.path == "/api/judge" && req.method == "POST") {
        resp = handlePostJudge(req.body);
    } else if (req.path == "/api/version-control/commit" && req.method == "POST") {
        resp = handlePostCommit(req.body);
    } else {
        // Serve static file
        resp = handleStaticFile(req.path);
    }

    std::string rawResponse = buildResponse(resp);
    send(clientSocket, rawResponse.c_str(), static_cast<int>(rawResponse.size()), 0);
    closesocket(clientSocket);
}

bool HttpServer::start() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed." << std::endl;
        return false;
    }

    listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed." << std::endl;
        WSACleanup();
        return false;
    }

    int opt = 1;
    setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(static_cast<u_short>(port));

    if (bind(listenSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed on port " << port << std::endl;
        closesocket(listenSocket);
        WSACleanup();
        return false;
    }

    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Listen failed." << std::endl;
        closesocket(listenSocket);
        WSACleanup();
        return false;
    }

    isRunning = true;
    std::cout << "=====================================================" << std::endl;
    std::cout << " DevCollab Pro - Native C++ Server Running!          " << std::endl;
    std::cout << " URL: http://localhost:" << port << "                " << std::endl;
    std::cout << " Serving Web UI from: " << publicDir << std::endl;
    std::cout << " 100% C++ Backend (Zero Python)                      " << std::endl;
    std::cout << "=====================================================" << std::endl;

    while (isRunning) {
        sockaddr_in clientAddr;
        int clientLen = sizeof(clientAddr);
        SOCKET clientSocket = accept(listenSocket, (sockaddr*)&clientAddr, &clientLen);

        if (clientSocket == INVALID_SOCKET) {
            if (!isRunning) break;
            continue;
        }

        ThreadParams* tp = new ThreadParams{this, clientSocket};
        HANDLE hThread = CreateThread(NULL, 0, clientThreadRouter, tp, 0, NULL);
        if (hThread != NULL) {
            CloseHandle(hThread); // Detach thread
        } else {
            handleClient(clientSocket);
            delete tp;
        }
    }

    return true;
}

void HttpServer::stop() {
    isRunning = false;
    if (listenSocket != INVALID_SOCKET) {
        closesocket(listenSocket);
        listenSocket = INVALID_SOCKET;
    }
    WSACleanup();
}

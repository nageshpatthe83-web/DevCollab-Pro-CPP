#include <iostream>
#include <string>
#include <windows.h>
#include "../include/HttpServer.h"
#include "../include/ConfigManager.h"

int main(int argc, char* argv[]) {
    int port = 8080;
    if (argc > 1) {
        try {
            port = std::stoi(argv[1]);
        } catch (...) {
            port = 8080;
        }
    }

    // Determine path to public directory
    char exePath[MAX_PATH];
    GetModuleFileNameA(NULL, exePath, MAX_PATH);
    std::string currentDir(exePath);
    size_t lastSlash = currentDir.find_last_of("\\/");
    if (lastSlash != std::string::npos) {
        currentDir = currentDir.substr(0, lastSlash);
    }

    std::string publicDir = currentDir + "\\..\\public";
    // Check if public folder exists in currentDir (e.g. if run directly from build or root)
    DWORD attrib = GetFileAttributesA(publicDir.c_str());
    if (attrib == INVALID_FILE_ATTRIBUTES || !(attrib & FILE_ATTRIBUTE_DIRECTORY)) {
        publicDir = currentDir + "\\public";
    }

    ConfigManager::getInstance().setPort(port);

    std::cout << "\n[INIT] Starting DevCollab Pro Native C++ Backend..." << std::endl;
    std::cout << "[OOP]  Design Patterns Loaded: Singleton, Factory, Strategy, Visitor, Observer" << std::endl;
    std::cout << "[DSA]  Empirical Big-O Profiler and Leaderboard Max-Heap Ready" << std::endl;
    std::cout << "[DBMS] CodeVault Version Control & Delta Storage Initialized\n" << std::endl;

    HttpServer server(port, publicDir);
    if (!server.start()) {
        std::cerr << "[ERROR] Server failed to start." << std::endl;
        return 1;
    }

    return 0;
}

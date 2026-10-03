@echo off
echo ===================================================
echo  Compiling DevCollab Pro Native C++ Backend
echo  Standard: C++14 ^| Libraries: ws2_32
echo ===================================================

g++ -std=c++14 -O2 -Iinclude src/ConfigManager.cpp src/CodeVault.cpp src/CodeSentry.cpp src/AlgoPro.cpp src/ScoringEngine.cpp src/Database.cpp src/HttpServer.cpp src/main.cpp -o devcollab_server.exe -lws2_32

if %ERRORLEVEL% EQU 0 (
    echo [SUCCESS] devcollab_server.exe compiled successfully!
) else (
    echo [ERROR] Compilation failed.
)

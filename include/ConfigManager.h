#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <string>
#include <map>

// Singleton Pattern for global configuration management
class ConfigManager {
private:
    static ConfigManager* instance;
    std::map<std::string, double> scoringWeights;
    int serverPort;
    std::string platformName;
    std::string version;

    // Private constructor for Singleton
    ConfigManager();

public:
    // Delete copy and assignment operations
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;

    static ConfigManager& getInstance();

    int getPort() const { return serverPort; }
    void setPort(int port) { serverPort = port; }

    std::string getPlatformName() const { return platformName; }
    std::string getVersion() const { return version; }

    double getWeight(const std::string& key) const;
    void setWeight(const std::string& key, double val);
    std::map<std::string, double> getAllWeights() const;
};

#endif // CONFIG_MANAGER_H

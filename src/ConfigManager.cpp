#include "../include/ConfigManager.h"

ConfigManager* ConfigManager::instance = nullptr;

ConfigManager::ConfigManager() {
    serverPort = 8080;
    platformName = "DevCollab Pro - C++ Edition";
    version = "2.0.0-CPP-Native";

    // Multi-dimensional scoring weights
    scoringWeights["functionality"] = 0.30;
    scoringWeights["codeQuality"]   = 0.15;
    scoringWeights["performance"]   = 0.10;
    scoringWeights["database"]      = 0.10;
    scoringWeights["documentation"] = 0.10;
    scoringWeights["security"]      = 0.05;
    scoringWeights["innovation"]    = 0.10;
    scoringWeights["uiUx"]          = 0.05;
    scoringWeights["presentation"]  = 0.05;
}

ConfigManager& ConfigManager::getInstance() {
    if (instance == nullptr) {
        instance = new ConfigManager();
    }
    return *instance;
}

double ConfigManager::getWeight(const std::string& key) const {
    auto it = scoringWeights.find(key);
    if (it != scoringWeights.end()) {
        return it->second;
    }
    return 0.0;
}

void ConfigManager::setWeight(const std::string& key, double val) {
    scoringWeights[key] = val;
}

std::map<std::string, double> ConfigManager::getAllWeights() const {
    return scoringWeights;
}

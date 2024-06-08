// ConfigManager.cpp
#include "ConfigManager.h"
#include "JsonManager.h"
#include "Printer.h"
#include <iostream>
#include <fstream>

using json = nlohmann::json;

ConfigManager* ConfigManager::instance = nullptr;

ConfigManager& ConfigManager::getInstance(const std::string& configFile) {
    if (!instance) {
        instance = new ConfigManager(configFile);
    }
    return *instance;
}

ConfigManager::ConfigManager(const std::string& configFile) {
    // Load the configuration file using JsonManager
    std::string configFilePath = getConfigFilePath(configFile); //Get or try to construct file path
    if (!configFilePath.empty() && std::ifstream(configFilePath).good()) { //If the file exists
        JsonManager& jsonManager = JsonManager::getInstance(configFilePath);

        // Access the configuration using JsonManager's getConfig() method
        config = jsonManager.getConfig(); // Save the configuration

        // Check if the "debug" section exists in the JSON
        if (config.contains("debug") && config["debug"].contains("verbosity")) {
            verbosity = config["debug"]["verbosity"].get<int>();
        } else {
            if (debug) {
                std::cerr << "Warning: Config file does not contain debug/verbosity section. Using default value." << std::endl;
            }
        }
    }
    else {
        if (debug) {
            std::cerr << "Warning: Invalid or missing configuration file: " << configFilePath << ". Using default configuration values." << std::endl;
        }
    }
}

int ConfigManager::getVerbosity() const {
    return verbosity;
}

const json& ConfigManager::getConfig() const {
    return config; // Return the JSON configuration
}

std::string ConfigManager::getConfigFilePath(const std::string& configFile) {
    std::string configFilePath;

    // Determine the configuration file path
    if (!configFile.empty()) {
        configFilePath = configFile;
    } else {
        std::string sourceDirectory = __FILE__;
        size_t lastSlash = sourceDirectory.find_last_of('/');
        if (lastSlash != std::string::npos) {
            sourceDirectory = sourceDirectory.substr(0, lastSlash);
            lastSlash = sourceDirectory.find_last_of('/');
            if (lastSlash != std::string::npos) {
                sourceDirectory = sourceDirectory.substr(0, lastSlash);
            }
        }
        configFilePath = sourceDirectory + "/config/config.json";
    }

    return configFilePath;
}

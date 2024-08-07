// ConfigManager.cpp
#include "ConfigManager.h"
#include "JsonManager.h"
#include "Printer.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib> // for getenv
#include <string>
#include <filesystem> // for filesystem utilities
#include <sys/stat.h> // for stat

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

        // Read the config and assign variables
        readConfig(config);
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
    return config;
}

int ConfigManager::getAveragePeriodJiffies() const {
    return averagePeriodJiffies;
}

int ConfigManager::getUpdatePeriodJiffies() const {
    return updatePeriodJiffies;
}


void ConfigManager::setVerbosity(int newVerbosity) {
    if (!config.contains("debug")) {
        config["debug"] = json::object(); // Create the debug section if it doesn't exist
    }
    verbosity = newVerbosity;
    config["debug"]["verbosity"] = newVerbosity;
}

void ConfigManager::setAveragePeriodJiffies(int averagePeriod) {
    if (!config.contains("system_info")) {
        config["average_period_jiffies"] = json::object();
    }
    averagePeriodJiffies = averagePeriod;
    config["system_info"]["average_period_jiffies"] = averagePeriod;
}

void ConfigManager::setUpdatePeriodJiffies(int updatePeriod) {
    if (!config.contains("system_info")) {
        config["update_period_jiffies"] = json::object();
    }
    updatePeriodJiffies = updatePeriod;
    config["system_info"]["update_period_jiffies"] = updatePeriod;
}

std::string ConfigManager::getConfigFilePath(const std::string& configFile) {
    std::string configFilePath;

    // Try to get the configuration file path from the environment variable
    const char* envConfigFilePath = std::getenv("SYSTEM_MONITOR_CONFIG_FILE_PATH");
    if (envConfigFilePath != nullptr) {
        if (fileExists(envConfigFilePath)) {
            configFilePath = envConfigFilePath;
            return configFilePath;
        }
    }

    // Check if the provided config file path exists
    if (!configFile.empty()) {
        if (fileExists(configFile)) {
            configFilePath = configFile;
            return configFilePath;
        }
    }

    // Determine the configuration file path from the source directory
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

    return configFilePath;
}

void ConfigManager::readConfig(const nlohmann::json& config) {
    readConfigSection(config, "debug.verbosity", verbosity, DEFAULT_VERBOSITY);
    readConfigSection(config, "system_info.update_period_jiffies", updatePeriodJiffies, DEFAULT_UPDATE_PERIOD_JIFFIES);
    readConfigSection(config, "system_info.average_period_jiffies", averagePeriodJiffies, DEFAULT_AVERAGE_PERIOD_JIFFIES);

}

template<typename T>
void ConfigManager::readConfigSection(const nlohmann::json& config, const std::string& configPath, T& target, const T& defaultValue) {
    target = getConfigValue<T>(config, configPath, defaultValue);
}

template<typename T>
T ConfigManager::getConfigValue(const nlohmann::json& config, const std::string& configPath, const T& defaultValue) {
    try {
        // Split the period-separated path into individual keys
        std::vector<std::string> keys;
        std::istringstream iss(configPath);
        std::string key;
        while (std::getline(iss, key, '.')) {
            keys.push_back(key);
        }

        // Traverse the JSON object using the keys
        const nlohmann::json* current = &config;
        for (const auto& k : keys) {
            current = &((*current)[k]);
        }

        // Return the value found
        return current->get<T>();
    } catch (const std::exception& e) {
        if (debug) {
            std::cerr << "Warning: Failed to read config value for path '" << configPath << "'. Using default value " << defaultValue << ". Exception: " << e.what() << std::endl;
        }
        return defaultValue;
    }
}

bool ConfigManager::fileExists(const std::string& path) {
    struct stat buffer;
    return (stat(path.c_str(), &buffer) == 0);
}

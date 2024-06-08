// ConfigManager.h
#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class ConfigManager {
public:
    static ConfigManager& getInstance(const std::string& configFile = "");
    int getVerbosity() const;
    const json& getConfig() const; // Method to get the JSON configuration

private:
    static ConfigManager* instance;
    json config;
    int verbosity = 0; // Default verbosity level

    ConfigManager(const std::string& configFile);
    void initializeVariables(const std::string& configFile);
    std::string getConfigFilePath(const std::string& configFile); // Private method to get the configuration file path
    bool debug = false; // Internal debug flag for config loading
};

#endif // CONFIG_MANAGER_H

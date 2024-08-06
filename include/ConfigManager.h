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
    int getUpdatePeriodJiffies() const;
    int getAveragePeriodJiffies() const;
    void setVerbosity(int verbosity);
    void setUpdatePeriodJiffies(int updatePeriod);
    void setAveragePeriodJiffies(int averagePeriod);
    const json& getConfig() const;

private:
    bool debug = false; // Internal debug flag for config loading

    static ConfigManager* instance;
    json config;
    int verbosity;
    int updatePeriodJiffies;
    int averagePeriodJiffies;

    // Default values
    const int DEFAULT_VERBOSITY = 0;
    const int DEFAULT_UPDATE_PERIOD_JIFFIES = 100;
    const int DEFAULT_AVERAGE_PERIOD_JIFFIES = 1000;

    //Methods
    ConfigManager(const std::string& configFile);
    void initializeVariables(const std::string& configFile);
    void readConfig(const nlohmann::json& config);
    std::string getConfigFilePath(const std::string& configFile); // Private method to get the configuration file path

    template<typename T>
    void readConfigSection(const nlohmann::json& config, const std::string& configSectionName, T& target, const T& defaultValue);
    template<typename T>
    T getConfigValue(const nlohmann::json& config, const std::string& configPath, const T& defaultValue);
    bool fileExists(const std::string& path);
};

#endif // CONFIG_MANAGER_H

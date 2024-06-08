// Printer.cpp
#include "Printer.h"
#include <iostream>
#include <chrono>
#include <ctime>

Printer::Printer() {
    // Initialize settings from the ConfigManager
    initializeSettings();
}

Printer& Printer::getInstance() {
    static Printer instance;
    return instance;
}

void Printer::initializeSettings() {
    // Get configuration from the ConfigManager
    const nlohmann::json& config = ConfigManager::getInstance().getConfig();
    verbosity = ConfigManager::getInstance().getVerbosity();

    // Extract other printing settings
    if (config.contains("printing")) {
        const nlohmann::json& printingConfig = config["printing"];
        printLineNumber = printingConfig.value("print_line_number", false);
        printCurrentTime = printingConfig.value("print_current_time", false);
        prefix = printingConfig.value("prefix", "");
        suffix = printingConfig.value("suffix", "");
        infoColor = printingConfig.value("info_color", "white");
        warningColor = printingConfig.value("warning_color", "yellow");
        errorColor = printingConfig.value("error_color", "red");
    } else {
        // Use default values if the printing section is not found
        printLineNumber = false;
        printCurrentTime = false;
        prefix = "";
        suffix = "";
        infoColor = "white";
        warningColor = "yellow";
        errorColor = "red";
    }
}


void Printer::print(const std::string& message, int lineNumber, const std::string& filename, int debugThreshold) const {
    if (verbosity < debugThreshold) {
        // If the verbosity level is below the debug threshold, don't print
        return;
    }
    printWithColor(message, "", lineNumber, filename, infoColor);
}

void Printer::printWarning(const std::string& message, int lineNumber, const std::string& filename, int debugThreshold) const {
    if (verbosity < debugThreshold) {
        // If the verbosity level is below the debug threshold, don't print
        return;
    }
    printWithColor(message, "WARNING", lineNumber, filename, warningColor);
}

void Printer::printError(const std::string& message, int lineNumber, const std::string& filename, int debugThreshold) const {
    if (verbosity < debugThreshold) {
        // If the verbosity level is below the debug threshold, don't print
        return;
    }
    printWithColor(message, "ERROR", lineNumber, filename, errorColor);
}

void Printer::printWithColor(const std::string& message, const std::string& status, int lineNumber, const std::string& filename, const std::string& color) const {
    std::string messageString = buildMessageString(message, status, lineNumber, filename);
    std::cout << colorizeString(messageString, color) << std::endl;
}

std::string Printer::buildMessageString(const std::string& message, const std::string& status, int lineNumber, const std::string& filename) const {
    std::string messageString;
    std::string trimmedFilename = filename;

    // Include the current time within curly braces before everything else
    if (printCurrentTime) {
        std::string currentTime = getCurrentTime();
        messageString += "{" + currentTime + "} ";
    }

    if (!prefix.empty() || !filename.empty() || lineNumber > 0) {
        messageString += "[";

        if (!prefix.empty()) {
            messageString += prefix;
        }

        if (!status.empty()) {
            messageString += ", " + status;
        }

        if (!filename.empty() && lineNumber > 0) {
            // Extract the filename from the path
            size_t lastSeparator = filename.find_last_of('/');
            if (lastSeparator != std::string::npos) {
                trimmedFilename = filename.substr(lastSeparator + 1);
            }

            messageString += " at (" + trimmedFilename + ":" + std::to_string(lineNumber) + ")";
        }

        messageString += "] ";
    }

    messageString += message;
    return messageString;
}


std::string Printer::getCurrentTime() const{
    auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    struct tm timeInfo;
    localtime_r(&now, &timeInfo);
    
    char timeBuffer[9]; // Room for "hh:mm:ss\0"
    strftime(timeBuffer, sizeof(timeBuffer), "%H:%M:%S", &timeInfo);
    
    return std::string(timeBuffer);
}


std::string Printer::colorizeString(const std::string& message, const std::string& color) const {
    std::string colorCode = getColorCode(color);
    std::string resetCode = "\033[0m";
    return colorCode + message + resetCode;
}

std::string Printer::getColorCode(const std::string& color) const {
    // Map color names to ANSI color codes
    std::unordered_map<std::string, std::string> colorMap = {
        {"black", "\033[30m"},
        {"red", "\033[31m"},
        {"green", "\033[32m"},
        {"yellow", "\033[33m"},
        {"blue", "\033[34m"},
        {"magenta", "\033[35m"},
        {"cyan", "\033[36m"},
        {"white", "\033[37m"}
    };

    if (colorMap.find(color) != colorMap.end()) {
        return colorMap[color];
    } else {
        return ""; // Return an empty string for unknown colors
    }
}

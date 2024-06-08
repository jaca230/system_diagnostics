// Printer.h
#ifndef PRINTER_H
#define PRINTER_H

#include "ConfigManager.h" // Include ConfigManager header file
#include <string>

class Printer {
public:
    static Printer& getInstance();

    void print(const std::string& message, int lineNumber = -1, const std::string& filename = "", int debugThreshold = -1) const;
    void printWarning(const std::string& message, int lineNumber = -1, const std::string& filename = "", int debugThreshold = -1) const;
    void printError(const std::string& message, int lineNumber = -1, const std::string& filename = "", int debugThreshold = -1) const;
    void printWithColor(const std::string& message, const std::string& status, int lineNumber, const std::string& filename, const std::string& color) const;

private:
    Printer();

    void initializeSettings();

    int verbosity;
    bool printLineNumber;
    bool printCurrentTime;
    std::string prefix;
    std::string suffix;
    std::string infoColor;
    std::string warningColor;
    std::string errorColor;
    std::string buildMessageString(const std::string& message, const std::string& status, int lineNumber, const std::string& filename) const;
    std::string colorizeString(const std::string& message, const std::string& color) const;
    std::string getColorCode(const std::string& color) const;
    std::string getCurrentTime() const;
};

#endif // PRINTER_H

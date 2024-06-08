#pragma once

#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#include <mutex>

class SystemInfo {
public:
    static SystemInfo& getInstance(); // Singleton access method
    ~SystemInfo(); // Destructor to close the file

    void updateSystemInfo(); // Method to update system information

    // Getters for system information
    double getCpuUsage() const;
    double getTotalRamMB() const;
    double getFreeRamMB() const;
    double getUsedRamMB() const;
    double getTotalRam() const;
    double getFreeRam() const;
    double getUsedRam() const;
    double getLoadAvg1Min() const;
    double getLoadAvg5Min() const;
    double getLoadAvg15Min() const;
    int getNumCores() const;

private:
    SystemInfo(); // Private constructor

    void initCpuUsage(); // Initialize CPU usage
    void initNumCores(); // Private method to initialize the number of CPU cores
    double calculatePercent(); //Calculate CPU usage percent

    std::ifstream statFile_; // File stream for /proc/stat
    unsigned long long lastTotalUser_, lastTotalUserLow_, lastTotalSys_, lastTotalIdle_;
    std::string cpuLabel_;
    double uptime_, totalRam_, freeRam_, usedRam_, loadAvg1Min_, loadAvg5Min_, loadAvg15Min_;
    double cpuUsagePercent_; // CPU usage percentage
    int numCores_; // Number of CPU cores

    static SystemInfo* instance_; // Singleton instance
    static std::mutex mutex_; // Mutex for thread safety
};

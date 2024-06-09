#pragma once

#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#include <mutex>
#include "CpuUsageCalculator.h"

class SystemInfo {
public:
    static SystemInfo& getInstance(); // Singleton access method
    ~SystemInfo(); // Destructor to close the file

    void updateSystemInfo(); // Method to update system information

    // Getters for system information
    double getCpuUsage() const;
    double getTimeStep() const;
    double getCpuUsageForCore(int core) const;
    double getTimeStepForCore(int core) const;
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
    void setCpuUsageResult(); //Private method to set CPU Usage statistics using CpuUsageCalculator

    std::ifstream statFile_; // File stream for /proc/stat
    unsigned long long lastTotalUser_, lastTotalUserLow_, lastTotalSys_, lastTotalIdle_;
    std::string cpuLabel_;
    double uptime_, totalRam_, freeRam_, usedRam_, loadAvg1Min_, loadAvg5Min_, loadAvg15Min_;
    int numCores_; // Number of CPU cores
    std::map<int, CpuUsageResult> coreUsageResults_; // Map to store CPU usage results for each core (total usage stored at -1)

    static SystemInfo* instance_; // Singleton instance
    static std::mutex mutex_; // Mutex for thread safety
};

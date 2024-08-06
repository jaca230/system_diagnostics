#pragma once

#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#include <mutex>
#include <vector>
#include <map>
#include <atomic>
#include "CpuUsageCalculator.h"

struct SystemInfoData {
    long total_ram;
    long free_ram;
    long total_ram_MB;
    long free_ram_MB;
    double cpu_usage_percent;
    int cpu_num_processors;
    double cpu_real_time_step;
    std::vector<double> cpu_usage_percent_per_core;
    std::vector<double> cpu_real_time_step_per_core;
    double load_avg_1min;
    double load_avg_5min;
    double load_avg_15min;
    double time_stamp_ns;
};

class SystemInfo {
public:
    static SystemInfo& getInstance(); // Singleton access method
    ~SystemInfo(); // Destructor to close the file

    void updateSystemInfo(); // Method to update system information

    //Public Getters (thread safe), get all information in on struct (SystemInfoData)
    SystemInfoData collectSystemInfo();
    std::vector<double> packageSystemInfoForMIDAS();

    void startPeriodicUpdates();
    void stopPeriodicUpdates();

private:
    SystemInfo(); // Private constructor

    //Private Getters (not thread safe)
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
    double getLastUpdateTimestampNanos() const; 

    void initCpuUsage(); // Initialize CPU usage
    void initNumCores(); // Private method to initialize the number of CPU cores
    void setCpuUsageResult(); //Private method to set CPU Usage statistics using CpuUsageCalculator
    void addDataPointToBuffer(); //Private method to add a data point to the buffer without computing usage results
    void initializeJiffiesInformation(); //Private method to grab system's definition of a jiffy
    unsigned long long getCurrentJiffy(); //Calculates the current jiffy from system information.

    void periodicUpdate();
    std::thread updateThread_;
    std::atomic<bool> running_;
    std::mutex updateMutex_;

    std::ifstream statFile_; // File stream for /proc/stat
    unsigned long long lastTotalUser_, lastTotalUserLow_, lastTotalSys_, lastTotalIdle_;
    std::string cpuLabel_;
    double uptime_, totalRam_, freeRam_, usedRam_, loadAvg1Min_, loadAvg5Min_, loadAvg15Min_;
    int numCores_; // Number of CPU cores
    std::map<int, CpuUsageResult> coreUsageResults_; // Map to store CPU usage results for each core (total usage stored at -1)

    unsigned long long jiffiesPerSecond_; //System jiffies per second
    unsigned long long updatePeriodJiffies_; //Number of jiffies per CPU sample
    unsigned long long averagePeriodJiffies_; //Number of jiffies average over to compute CPU usage statistics
    unsigned long long lastUpdateJiffies_ = 0; //Number of jiffies before last update, intially zero

    static SystemInfo* instance_; // Singleton instance
    static std::mutex mutex_; // Mutex for thread safety
    mutable std::mutex dataMutex_;  // Mutex for synchronizing access to member variables

    std::chrono::time_point<std::chrono::high_resolution_clock> lastUpdate_; //Unix timestamp
};

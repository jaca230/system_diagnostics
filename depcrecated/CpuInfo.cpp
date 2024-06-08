#include "CpuInfo.h"
#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>

CpuInfo::CpuInfo() {
    std::cout << "Initializing CPU usage..." << std::endl;
    InitCpuUsage();
    std::cout << "CPU usage initialized." << std::endl;
}

void CpuInfo::InitCpuUsage() const {
    FILE* file = fopen("/proc/stat", "r");
    if (file) {
        fscanf(file, "cpu %llu %llu %llu %llu", &lastTotalUser_, &lastTotalUserLow_,
               &lastTotalSys_, &lastTotalIdle_);
        fclose(file);
        std::cout << "InitCpuUsage - lastTotalUser_: " << lastTotalUser_
                  << ", lastTotalUserLow_: " << lastTotalUserLow_
                  << ", lastTotalSys_: " << lastTotalSys_
                  << ", lastTotalIdle_: " << lastTotalIdle_ << std::endl;
    } else {
        std::cout << "Failed to open /proc/stat for initialization." << std::endl;
    }
}

void CpuInfo::GetCpuInfo() {
    struct sysinfo sys_info;
    if (sysinfo(&sys_info) == 0) {
        uptime_ = sys_info.uptime;
        totalram_ = sys_info.totalram / 1024 / 1024;  // Convert to MB
        load_avg_[0] = sys_info.loads[0] / 65536.0;
        load_avg_[1] = sys_info.loads[1] / 65536.0;
        load_avg_[2] = sys_info.loads[2] / 65536.0;
        
        std::cout << "GetCpuInfo - uptime_: " << uptime_
                  << ", totalram_: " << totalram_
                  << ", load_avg_[0]: " << load_avg_[0]
                  << ", load_avg_[1]: " << load_avg_[1]
                  << ", load_avg_[2]: " << load_avg_[2] << std::endl;
    } else {
        std::cout << "Failed to get system information." << std::endl;
    }
}

long CpuInfo::GetTotalRamMB() const {
    std::cout << "GetTotalRamMB - totalram_: " << totalram_ << std::endl;
    return totalram_;
}

long CpuInfo::GetFreeRamMB() const {
    struct sysinfo sys_info;
    if (sysinfo(&sys_info) == 0) {
        long free_ram = sys_info.freeram / 1024 / 1024;  // Convert to MB
        std::cout << "GetFreeRamMB - free_ram: " << free_ram << std::endl;
        return free_ram;
    } else {
        std::cout << "Failed to get free RAM." << std::endl;
        return -1;  // Error handling
    }
}

double CpuInfo::GetLoadAvg1Min() const {
    std::cout << "GetLoadAvg1Min - load_avg_[0]: " << load_avg_[0] << std::endl;
    return load_avg_[0];
}

double CpuInfo::GetLoadAvg5Min() const {
    std::cout << "GetLoadAvg5Min - load_avg_[1]: " << load_avg_[1] << std::endl;
    return load_avg_[1];
}

double CpuInfo::GetLoadAvg15Min() const {
    std::cout << "GetLoadAvg15Min - load_avg_[2]: " << load_avg_[2] << std::endl;
    return load_avg_[2];
}

double CpuInfo::GetCpuUsage() const {
    double percent;
    FILE* file;
    unsigned long long totalUser, totalUserLow, totalSys, totalIdle, total;
    std::this_thread::sleep_for(std::chrono::seconds(1));
    file = fopen("/proc/stat", "r");
    if (file) {
        fscanf(file, "cpu %llu %llu %llu %llu", &totalUser, &totalUserLow,
               &totalSys, &totalIdle);
        fclose(file);

        std::cout << "GetCpuUsage - Read values: totalUser: " << totalUser
                  << ", totalUserLow: " << totalUserLow
                  << ", totalSys: " << totalSys
                  << ", totalIdle: " << totalIdle << std::endl;

        if (totalUser < lastTotalUser_ || totalUserLow < lastTotalUserLow_ ||
            totalSys < lastTotalSys_ || totalIdle < lastTotalIdle_) {
            // Overflow detection. Just skip this value.
            percent = -1.0;
            std::cout << "GetCpuUsage - Overflow detected." << std::endl;
        } else {
            total = (totalUser - lastTotalUser_) + (totalUserLow - lastTotalUserLow_) +
                    (totalSys - lastTotalSys_);
            std::cout << "GetCpuUsage - Computed intermediate total: " << total << std::endl;
            percent = total;
            total += (totalIdle - lastTotalIdle_);
            percent /= total;
            percent *= 100;

            std::cout << "GetCpuUsage - Computed values: total: " << total
                      << ", percent: " << percent << std::endl;
        }

        lastTotalUser_ = totalUser;
        lastTotalUserLow_ = totalUserLow;
        lastTotalSys_ = totalSys;
        lastTotalIdle_ = totalIdle;

        std::cout << "GetCpuUsage - Updated values: lastTotalUser_: " << lastTotalUser_
                  << ", lastTotalUserLow_: " << lastTotalUserLow_
                  << ", lastTotalSys_: " << lastTotalSys_
                  << ", lastTotalIdle_: " << lastTotalIdle_ << std::endl;
    } else {
        percent = -1.0;  // Error handling
        std::cout << "Failed to open /proc/stat for CPU usage." << std::endl;
    }

    return percent;
}

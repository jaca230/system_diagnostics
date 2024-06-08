#include "SystemInfo.h"
#include "Printer.h"
#include <sys/sysinfo.h>

SystemInfo* SystemInfo::instance_ = nullptr;
std::mutex SystemInfo::mutex_;

SystemInfo& SystemInfo::getInstance() {
    // Thread-safe singleton creation
    std::lock_guard<std::mutex> lock(mutex_);
    if (!instance_) {
        instance_ = new SystemInfo();
    }
    return *instance_;
}

SystemInfo::SystemInfo() {
    initCpuUsage();
    initNumCores();
}

void SystemInfo::initCpuUsage() {
    Printer& printer = Printer::getInstance();
    printer.print("Initializing CPU usage...", -1, "", 2);
    // Open /proc/stat file
    statFile_.open("/proc/stat");
    if (statFile_.is_open()) {   
        // Read CPU statistics from /proc/stat
        statFile_ >> cpuLabel_ >> lastTotalUser_ >> lastTotalUserLow_ >> lastTotalSys_ >> lastTotalIdle_;
        printer.print("InitCpuUsage - lastTotalUser_: " + std::to_string(lastTotalUser_) +
                                      ", lastTotalUserLow_: " + std::to_string(lastTotalUserLow_) +
                                      ", lastTotalSys_: " + std::to_string(lastTotalSys_) +
                                      ", lastTotalIdle_: " + std::to_string(lastTotalIdle_), -1, "", 2);
        printer.print("CPU usage initialized.", -1, "", 2); 
    } else {
        printer.printWarning("Failed to open /proc/stat for initialization.", __LINE__, __FILE__, -1);
    }
}

void SystemInfo::updateSystemInfo() {
    Printer& printer = Printer::getInstance();
    struct sysinfo sys_info;
    if (sysinfo(&sys_info) == 0) {
        uptime_ = sys_info.uptime;
        totalRam_ = sys_info.totalram;  
        freeRam_ = sys_info.freeram;
        usedRam_ = (sys_info.totalram - sys_info.freeram);
        loadAvg1Min_ = sys_info.loads[0] / 65536.0;
        loadAvg5Min_ = sys_info.loads[1] / 65536.0;
        loadAvg15Min_ = sys_info.loads[2] / 65536.0;
        cpuUsagePercent_ = calculatePercent();
    } else {
        printer.printWarning("Failed to update CPU information.", __LINE__, __FILE__, -1);
    }
}

double SystemInfo::getCpuUsage() const {
    return cpuUsagePercent_;
}

double SystemInfo::getTotalRamMB() const {
    return totalRam_ / 1024 /1024; // Convert to MB
}

double SystemInfo::getFreeRamMB() const {
    return freeRam_ / 1024 /1024; // Convert to MB;
}

double SystemInfo::getUsedRamMB() const {
    return usedRam_ / 1024 /1024; // Convert to MB;
}

double SystemInfo::getTotalRam() const {
    return totalRam_;
}

double SystemInfo::getFreeRam() const {
    return freeRam_;
}

double SystemInfo::getUsedRam() const {
    return usedRam_; 
}

double SystemInfo::getLoadAvg1Min() const {
    return loadAvg1Min_;
}

double SystemInfo::getLoadAvg5Min() const {
    return loadAvg5Min_;
}

double SystemInfo::getLoadAvg15Min() const {
    return loadAvg15Min_;
}

int SystemInfo::getNumCores() const {
    return numCores_;
}

double SystemInfo::calculatePercent() {
    Printer& printer = Printer::getInstance(); // Get instance of Printer
    double percent;
    unsigned long long totalUser, totalUserLow, totalSys, totalIdle, total;

    // Read CPU statistics from /proc/stat using the stream
    statFile_.seekg(0); // Move stream cursor to the beginning of the file
    statFile_ >> cpuLabel_ >> totalUser >> totalUserLow >> totalSys >> totalIdle;

    printer.print("CalculatePercent - Read values: totalUser: " + std::to_string(totalUser) +
                                     ", totalUserLow: " + std::to_string(totalUserLow) +
                                     ", totalSys: " + std::to_string(totalSys) +
                                     ", totalIdle: " + std::to_string(totalIdle), -1, "", 2);

    if (totalUser < lastTotalUser_ || totalUserLow < lastTotalUserLow_ ||
        totalSys < lastTotalSys_ || totalIdle < lastTotalIdle_) {
        // Overflow detection. Just skip this value.
        percent = -1.0;
        printer.print("CalculatePercent - Overflow detected.");
    } else {
        total = (totalUser - lastTotalUser_) + (totalUserLow - lastTotalUserLow_) +
                (totalSys - lastTotalSys_);
        printer.print("CalculatePercent - Computed intermediate total: " + std::to_string(total), -1, "", 2);
        percent = total;
        total += (totalIdle - lastTotalIdle_);
        percent /= total;
        percent *= 100;

        printer.print("CalculatePercent - Computed values: total: " + std::to_string(total) +
                                         ", percent: " + std::to_string(percent), -1, "", 2);
    }

    // Update the last values
    lastTotalUser_ = totalUser;
    lastTotalUserLow_ = totalUserLow;
    lastTotalSys_ = totalSys;
    lastTotalIdle_ = totalIdle;

    printer.print("CalculatePercent - Updated values: lastTotalUser_: " + std::to_string(lastTotalUser_) +
                                     ", lastTotalUserLow_: " + std::to_string(lastTotalUserLow_) +
                                     ", lastTotalSys_: " + std::to_string(lastTotalSys_) +
                                     ", lastTotalIdle_: " + std::to_string(lastTotalIdle_),-1, "", 2);

    return percent;
}

void SystemInfo::initNumCores() {
    std::ifstream cpuinfo("/proc/cpuinfo");
    if (cpuinfo.is_open()) {
        std::string line;
        while (std::getline(cpuinfo, line)) {
            if (line.substr(0, 9) == "processor") {
                numCores_++;
            }
        }
        cpuinfo.close();
    } else {
        Printer& printer = Printer::getInstance();
        printer.printWarning("Failed to open /proc/cpuinfo to get the number of CPU cores.", __LINE__, __FILE__, 1);
    }
}
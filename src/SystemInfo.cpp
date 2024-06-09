#include "SystemInfo.h"
#include "Printer.h"
#include "ConfigManager.h"
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
        printer.print("Initial stats: lastTotalUser_: " + std::to_string(lastTotalUser_) +
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
    } else {
        printer.printWarning("Failed to update CPU information.", __LINE__, __FILE__, -1);
    }
    // Calculate CPU usage for a specified duration in config
    setCpuUsageResult();
}

double SystemInfo::getCpuUsage() const {
    return getCpuUsageForCore(CpuUsageCalculator::TOTAL_CPU_USAGE_INDEX);
}

double SystemInfo::getTimeStep() const {
    return getTimeStepForCore(CpuUsageCalculator::TOTAL_CPU_USAGE_INDEX);
}

double SystemInfo::getCpuUsageForCore(int core) const {
    // Check if the core exists in the map
    auto it = coreUsageResults_.find(core);
    if (it != coreUsageResults_.end()) {
        // Return the CPU usage percentage for the specified core
        return it->second.usagePercent;
    } else {
        // Print a warning that the core wasn't found
        Printer& printer = Printer::getInstance();
        printer.printWarning("CPU usage for core " + std::to_string(core) + " not found.", __LINE__, __FILE__, -1);
        return -1.0; // Default value
    }
}

double SystemInfo::getTimeStepForCore(int core) const {
    // Check if the core exists in the map
    auto it = coreUsageResults_.find(core);
    if (it != coreUsageResults_.end()) {
        // Return the time step for the specified core
        return it->second.timeDiffSeconds;
    } else {
        // Print a warning that the core wasn't found
        Printer& printer = Printer::getInstance();
        printer.printWarning("Time step for core " + std::to_string(core) + " not found.", __LINE__, __FILE__, -1);
        return -1.0; // Default value
    }
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

void SystemInfo::initNumCores() {
    // Read the number of CPU cores from /proc/cpuinfo
    int numCoresCpuInfo = 0;
    std::ifstream cpuinfo("/proc/cpuinfo");
    if (cpuinfo.is_open()) {
        std::string line;
        while (std::getline(cpuinfo, line)) {
            if (line.substr(0, 9) == "processor") {
                numCoresCpuInfo++;
            }
        }
        cpuinfo.close();
    } else {
        Printer& printer = Printer::getInstance();
        printer.printWarning("Failed to open /proc/cpuinfo to get the number of CPU cores.", __LINE__, __FILE__, -1);
    }

    // Read the number of CPU cores from /proc/stat
    int numCoresProcStat = -1; //Start at -1 because we count the first line "cpu" which is just the total
    std::ifstream procstat("/proc/stat");
    if (procstat.is_open()) {
        std::string line;
        while (std::getline(procstat, line)) {
            if (line.substr(0, 3) == "cpu") {
                numCoresProcStat++;
            }
        }
        procstat.close();
    } else {
        Printer& printer = Printer::getInstance();
        printer.printWarning("Failed to open /proc/stat to get the number of CPU cores.", __LINE__, __FILE__, -1);
    }

    // Check if the number of cores matches
    if (numCoresProcStat != numCoresCpuInfo) {
        Printer& printer = Printer::getInstance();
        printer.printWarning("Number of CPU cores detected by /proc/cpuinfo is " + std::to_string(numCores_) + " and by /proc/stat is " + std::to_string(numCoresProcStat) + ".", __LINE__, __FILE__, -1);
    }

    // We will always chose to use /proc/stat
    numCores_ = numCoresProcStat;

}

void SystemInfo::setCpuUsageResult() {
    Printer& printer = Printer::getInstance(); //Initialize Printer for debug printing

    //Get the time average from config
    int cpuUsageTimeAverageMs = ConfigManager::getInstance().getCpuUsageTimeAverageMs();

    //Read info from file
    statFile_.seekg(0); // Move stream cursor to the beginning of the file
    
    // Read CPU usage information for the current core
    unsigned long long totalUser, totalUserLow, totalSys, totalIdle;
    statFile_ >> cpuLabel_ >> totalUser >> totalUserLow >> totalSys >> totalIdle;

    // Print debug information
    printer.print("Total CPU Info: lastTotalUser_: " + std::to_string(totalUser) +
        ", lastTotalUserLow_: " + std::to_string(totalUserLow) +
        ", lastTotalSys_: " + std::to_string(totalSys) +
        ", lastTotalIdle_: " + std::to_string(totalIdle), -1, "", 2);

    // Calculate CPU Usage for each core
    CpuUsageCalculator& cpuUsageCalculator = CpuUsageCalculator::getInstanceForTotal();
    cpuUsageCalculator.addDataPointForTotal(totalUser, totalUserLow, totalSys, totalIdle);

    // Calculate CPU usage result for the current core
    coreUsageResults_[CpuUsageCalculator::TOTAL_CPU_USAGE_INDEX] = cpuUsageCalculator.calculateCpuUsagePercentForTotal(std::chrono::milliseconds(cpuUsageTimeAverageMs));

    // Iterate over each core
    for (int core = 0; core < numCores_; ++core) {
        // Move cursor to the appropriate position for the current core
        statFile_.seekg(0);

        // Move cursor to the appropriate position for the current core
        for (int i = 0; i <= core; ++i) {
            std::string line;
            std::getline(statFile_, line); // Move cursor to the next line
        }

        // Read CPU usage information for the current core
        unsigned long long totalUser, totalUserLow, totalSys, totalIdle;
        statFile_ >> cpuLabel_ >> totalUser >> totalUserLow >> totalSys >> totalIdle;

        // Print debug information
        printer.print("For Core " + std::to_string(core) + ": lastTotalUser_: " + std::to_string(totalUser) +
            ", lastTotalUserLow_: " + std::to_string(totalUserLow) +
            ", lastTotalSys_: " + std::to_string(totalSys) +
            ", lastTotalIdle_: " + std::to_string(totalIdle), -1, "", 2);

        // Calculate CPU Usage for each core
        CpuUsageCalculator& cpuUsageCalculator = CpuUsageCalculator::getInstanceForCore(core);
        cpuUsageCalculator.addDataPointForCore(core,totalUser, totalUserLow, totalSys, totalIdle);

        // Calculate CPU usage result for the current core
        coreUsageResults_[core] = cpuUsageCalculator.calculateCpuUsagePercentForCore(core, std::chrono::milliseconds(cpuUsageTimeAverageMs));
    }
}

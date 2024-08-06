#include "SystemInfo.h"
#include "Printer.h"
#include "ConfigManager.h"
#include <sys/sysinfo.h>
#include <unistd.h>
#include <vector>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <fstream>
#include <ctime>
#include <thread>
#include <chrono>
#include <atomic>
#include <iostream>
#include <mutex>


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
    initializeJiffiesInformation();
    initNumCores();
    initCpuUsage();
}

void SystemInfo::startPeriodicUpdates() {
    //Stop any running threads.
    stopPeriodicUpdates();
    // Ensure only one thread is started
    std::cout << running_<< std::endl;
    if (running_) {
        return;
    }

    running_ = true;
    updateThread_ = std::thread([this]() {
        this->periodicUpdate();
    });
}

void SystemInfo::stopPeriodicUpdates() {
    // Stop the periodic update thread
    running_ = false;

    // Wait for the thread to finish
    if (updateThread_.joinable()) {
        updateThread_.join();
    }
}

void SystemInfo::periodicUpdate() {
    while (running_) {
        auto start = std::chrono::steady_clock::now();

        // Update system information
        this->updateSystemInfo();

        // Calculate the time to sleep
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed = end - start;
        double elapsedSeconds = elapsed.count();
        
        // Calculate sleep time based on updatePeriodJiffies_
        double updatePeriodSeconds = static_cast<double>(updatePeriodJiffies_) / jiffiesPerSecond_;
        std::chrono::duration<double> sleepDuration = std::chrono::duration<double>(updatePeriodSeconds - elapsedSeconds);

        // Sleep for the remaining duration
        if (sleepDuration.count() > 0) {
            std::this_thread::sleep_for(sleepDuration);
        } else {
            // If elapsed time exceeds the update period, log a warning or handle accordingly
            Printer::getInstance().printWarning("Update period exceeded by " + std::to_string(-sleepDuration.count()) + " seconds.", __LINE__, __FILE__, -1);
        }
    }
}


void SystemInfo::updateSystemInfo() {
    std::lock_guard<std::mutex> lock(dataMutex_);
    Printer& printer = Printer::getInstance();

    // Get the current time in jiffies
    unsigned long long currentJiffies = getCurrentJiffy();

    // Check if the update period has passed
    if (currentJiffies - lastUpdateJiffies_ >= updatePeriodJiffies_) {
        // Print a warning if the difference is greater than the update period
        if (lastUpdateJiffies_ != 0) {
            if (currentJiffies - lastUpdateJiffies_ > updatePeriodJiffies_) {
                printer.printWarning("Missed an update. Time since last update: " + std::to_string(currentJiffies - lastUpdateJiffies_) +
                                    " jiffies, which is greater than the update period of " + std::to_string(updatePeriodJiffies_) + " jiffies.", __LINE__, __FILE__, 2);
            }
         }

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

        // Update the last update time
        lastUpdateJiffies_ = currentJiffies;
    }
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
    // Calculate CPU usage for a specified duration in config
    addDataPointToBuffer();
}

void SystemInfo::initializeJiffiesInformation() {
    Printer& printer = Printer::getInstance();
    printer.print("Initializing jiffies per second...", -1, "", 2);

    errno = 0; // Reset errno before sysconf
    long tempJiffies = sysconf(_SC_CLK_TCK);

    if (tempJiffies == -1) {
        // sysconf failed, handle the error
        printer.printError("Error fetching jiffies per second: " + std::string(strerror(errno)), __LINE__, __FILE__, -1);
        // Use default value
        jiffiesPerSecond_ = 100ULL;
        printer.printWarning("Fallback to default value: " + std::to_string(jiffiesPerSecond_), __LINE__, __FILE__, -1);
    } else {
        // Ensure tempJiffies is correctly cast to unsigned long long
        jiffiesPerSecond_ = static_cast<unsigned long long>(tempJiffies);
        printer.print("Jiffies per second: " + std::to_string(jiffiesPerSecond_), -1, "", 2);
    }

    updatePeriodJiffies_ = static_cast<unsigned long long>(ConfigManager::getInstance().getUpdatePeriodJiffies());
    averagePeriodJiffies_ = static_cast<unsigned long long>(ConfigManager::getInstance().getAveragePeriodJiffies());

    // Check if average period is shorter than update period
    if (averagePeriodJiffies_ < updatePeriodJiffies_) {
        printer.printWarning(
            "Warning: Average period (" + std::to_string(averagePeriodJiffies_) +
            ") is shorter than update period (" + std::to_string(updatePeriodJiffies_) + ").",
            __LINE__, __FILE__, -1
        );
    }

    // Check if average period is not divisible by update period
    if (updatePeriodJiffies_ != 0 && averagePeriodJiffies_ % updatePeriodJiffies_ != 0) {
        printer.printWarning(
            "Warning: Average period (" + std::to_string(averagePeriodJiffies_) +
            ") is not divisible by update period (" + std::to_string(updatePeriodJiffies_) + ").",
            __LINE__, __FILE__, -1
        );
    }
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
        double secondsPassed = static_cast<double>(it->second.jiffiesPassed) / jiffiesPerSecond_;
        return secondsPassed;
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

double SystemInfo::getLastUpdateTimestampNanos() const {
    // Convert lastUpdate_ to nanoseconds since the Unix epoch
    auto duration = lastUpdate_.time_since_epoch();
    return static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count());
}

SystemInfoData SystemInfo::collectSystemInfo() {
    std::lock_guard<std::mutex> lock(dataMutex_);
    SystemInfoData data;

    // Populate data structure
    data.total_ram = this->getTotalRam();
    data.free_ram = this->getFreeRam();
    data.total_ram_MB = this->getTotalRamMB();
    data.free_ram_MB = this->getFreeRamMB();
    data.cpu_usage_percent = this->getCpuUsage();
    data.cpu_num_processors = this->getNumCores();
    data.cpu_real_time_step = this->getTimeStep();

    // Gather per-core data
    data.cpu_usage_percent_per_core.resize(data.cpu_num_processors);
    data.cpu_real_time_step_per_core.resize(data.cpu_num_processors);
    for (int core = 0; core < data.cpu_num_processors; ++core) {
        data.cpu_usage_percent_per_core[core] = this->getCpuUsageForCore(core);
        data.cpu_real_time_step_per_core[core] = this->getTimeStepForCore(core);
    }

    // Load averages
    data.load_avg_1min = this->getLoadAvg1Min();
    data.load_avg_5min = this->getLoadAvg5Min();
    data.load_avg_15min = this->getLoadAvg15Min();

    //Timestamp
    data.time_stamp_ns = getLastUpdateTimestampNanos();

    return data;
}

std::vector<double> SystemInfo::packageSystemInfoForMIDAS() {
    SystemInfoData data = this->collectSystemInfo();
    std::vector<double> packagedData;

    packagedData.push_back(static_cast<double>(0)); //Initialize with 0
    packagedData.push_back(static_cast<double>(data.time_stamp_ns));
    packagedData.push_back(static_cast<double>(data.total_ram));
    packagedData.push_back(static_cast<double>(data.free_ram));
    packagedData.push_back(data.load_avg_1min);
    packagedData.push_back(data.load_avg_5min);
    packagedData.push_back(data.load_avg_15min);
    packagedData.push_back(data.cpu_usage_percent);
    packagedData.push_back(data.cpu_real_time_step);
    packagedData.push_back(static_cast<double>(data.cpu_num_processors));
    for (int core = 0; core < data.cpu_num_processors; ++core) {
        packagedData.push_back(data.cpu_usage_percent_per_core[core]);
        packagedData.push_back(data.cpu_real_time_step_per_core[core]);
    }
    packagedData[0] = static_cast<double>(packagedData.size()-1); 

    return packagedData;
}

void SystemInfo::setCpuUsageResult() {
    Printer& printer = Printer::getInstance(); // Initialize Printer for debug printing

    // Check if the file is open
    if (!statFile_.is_open()) {
        printer.print("Error: statFile_ is not open.", -1, "", 2);
        return;
    }

    // Move the stream cursor to the beginning of the file
    statFile_.clear(); // Clear any EOF or fail flags
    statFile_.seekg(0);

    // Read total CPU usage information (first line)
    std::string line;
    if (!std::getline(statFile_, line)) {
        printer.print("Error: Failed to read the first line from the file.", -1, "", 2);
        return;
    }

    std::istringstream totalCpuStream(line);
    std::string cpuLabel;
    unsigned long long totalUser, totalUserLow, totalSys, totalIdle;
    totalCpuStream >> cpuLabel >> totalUser >> totalUserLow >> totalSys >> totalIdle;

    // Get the current jiffy
    unsigned long long currentJiffy = getCurrentJiffy();

    // Calculate CPU Usage for the total CPU
    CpuUsageCalculator& cpuUsageCalculator = CpuUsageCalculator::getInstanceForTotal();
    cpuUsageCalculator.addDataPointForTotal(totalUser, totalUserLow, totalSys, totalIdle, currentJiffy);

    // Calculate CPU usage result for the total CPU
    coreUsageResults_[CpuUsageCalculator::TOTAL_CPU_USAGE_INDEX] = cpuUsageCalculator.calculateCpuUsagePercentForTotal();

    // Read lines for each core, up to numCores_
    std::vector<std::string> coreLines(numCores_); // Allocate space for core lines
    int coreCount = 0;
    
    while (coreCount < numCores_ && std::getline(statFile_, line)) {
        coreLines[coreCount] = line;
        ++coreCount;
    }

    // Iterate over each core
    for (int core = 0; core < numCores_; ++core) {
        if (core >= coreLines.size() || coreLines[core].empty()) {
            printer.print("Core " + std::to_string(core) + " data is not available.", -1, "", 2);
            continue;
        }

        // Parse the line for the current core
        std::istringstream coreStream(coreLines[core]);
        coreStream >> cpuLabel >> totalUser >> totalUserLow >> totalSys >> totalIdle;

        // Calculate CPU Usage for each core
        CpuUsageCalculator& cpuUsageCalculator = CpuUsageCalculator::getInstanceForCore(core);
        cpuUsageCalculator.addDataPointForCore(core, totalUser, totalUserLow, totalSys, totalIdle, currentJiffy);

        // Calculate CPU usage result for the current core
        coreUsageResults_[core] = cpuUsageCalculator.calculateCpuUsagePercentForCore(core);
    }
    
    lastUpdate_ = std::chrono::high_resolution_clock::now(); // Use high_resolution_clock for unix timestamp

}

void SystemInfo::addDataPointToBuffer() {
    Printer& printer = Printer::getInstance(); // Initialize Printer for debug printing

    // Check if the file is open
    if (!statFile_.is_open()) {
        printer.print("Error: statFile_ is not open.", -1, "", 2);
        return;
    }

    // Move the stream cursor to the beginning of the file
    statFile_.clear(); // Clear any EOF or fail flags
    statFile_.seekg(0);

    // Read total CPU usage information (first line)
    std::string line;
    if (!std::getline(statFile_, line)) {
        printer.print("Error: Failed to read the first line from the file.", -1, "", 2);
        return;
    }

    std::istringstream totalCpuStream(line);
    std::string cpuLabel;
    unsigned long long totalUser, totalUserLow, totalSys, totalIdle;
    totalCpuStream >> cpuLabel >> totalUser >> totalUserLow >> totalSys >> totalIdle;

    // Get the current jiffy
    unsigned long long currentJiffy = getCurrentJiffy();

    // Calculate CPU Usage for the total CPU
    CpuUsageCalculator& cpuUsageCalculator = CpuUsageCalculator::getInstanceForTotal();
    cpuUsageCalculator.addDataPointForTotal(totalUser, totalUserLow, totalSys, totalIdle, currentJiffy);

    // Read lines for each core, up to numCores_
    std::vector<std::string> coreLines(numCores_); // Allocate space for core lines
    int coreCount = 0;
    
    while (coreCount < numCores_ && std::getline(statFile_, line)) {
        coreLines[coreCount] = line;
        ++coreCount;
    }

    // Iterate over each core
    for (int core = 0; core < numCores_; ++core) {
        if (core >= coreLines.size() || coreLines[core].empty()) {
            printer.print("Core " + std::to_string(core) + " data is not available.", -1, "", 2);
            continue;
        }

        // Parse the line for the current core
        std::istringstream coreStream(coreLines[core]);
        coreStream >> cpuLabel >> totalUser >> totalUserLow >> totalSys >> totalIdle;

        // Calculate CPU Usage for each core
        CpuUsageCalculator& cpuUsageCalculator = CpuUsageCalculator::getInstanceForCore(core);
        cpuUsageCalculator.addDataPointForCore(core, totalUser, totalUserLow, totalSys, totalIdle, currentJiffy);

    }
}



unsigned long long SystemInfo::getCurrentJiffy() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);

    // Convert time to jiffies
    unsigned long long currentJiffies = ts.tv_sec * jiffiesPerSecond_;
    currentJiffies += ts.tv_nsec / (1000000000 / jiffiesPerSecond_);

    return currentJiffies;
}

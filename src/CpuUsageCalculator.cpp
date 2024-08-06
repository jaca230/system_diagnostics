#include "CpuUsageCalculator.h"
#include "ConfigManager.h"
#include "Printer.h"
#include <cmath> // For std::ceil function

// Initialize static variables
std::map<int, CpuUsageCalculator*> CpuUsageCalculator::coreInstances_;
std::mutex CpuUsageCalculator::mutex_;
const int CpuUsageCalculator::TOTAL_CPU_USAGE_INDEX = -1;

CpuUsageCalculator::CpuUsageCalculator() {
    // Retrieve jiffies values from ConfigManager
    unsigned long long averagePeriodJiffies = static_cast<unsigned long long>(ConfigManager::getInstance().getAveragePeriodJiffies());
    unsigned long long updatePeriodJiffies = static_cast<unsigned long long>(ConfigManager::getInstance().getUpdatePeriodJiffies());

    // Calculate buffer size
    if (updatePeriodJiffies == 0) {
        // Handle potential division by zero if updatePeriodJiffies is zero
        throw std::runtime_error("Update period jiffies cannot be zero.");
    }

    // Calculate the buffer size
    bufferSize_ = static_cast<std::size_t>(std::ceil(static_cast<double>(averagePeriodJiffies) / updatePeriodJiffies)) + 1;

    // Initialize buffer with calculated size
    //buffer_.resize(bufferSize_);
    currentIndex_ = 0;
    largestOccupiedIndex_ = 0;
}

CpuUsageCalculator::~CpuUsageCalculator() {
    // Destructor implementation
}

CpuUsageCalculator& CpuUsageCalculator::getInstanceForCore(int core) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (coreInstances_.find(core) == coreInstances_.end()) {
        coreInstances_[core] = new CpuUsageCalculator();
    }
    return *coreInstances_[core];
}

CpuUsageCalculator& CpuUsageCalculator::getInstanceForTotal() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (coreInstances_.find(TOTAL_CPU_USAGE_INDEX) == coreInstances_.end()) {
        coreInstances_[TOTAL_CPU_USAGE_INDEX] = new CpuUsageCalculator();
    }
    return *coreInstances_[TOTAL_CPU_USAGE_INDEX];
}

void CpuUsageCalculator::addDataPointForCore(int core, unsigned long long totalUser, unsigned long long totalUserLow,
                                              unsigned long long totalSys, unsigned long long totalIdle, unsigned long long jiffies) {
    std::lock_guard<std::mutex> lock(mutex_); // Lock for multithreading

    DataPoint newDataPoint = {totalUser, totalUserLow, totalSys, totalIdle, jiffies}; // Create data point

    // Check if buffer is full
    if (buffer_.size() >= bufferSize_) {
        // Remove the oldest data point (FIFO behavior)
        buffer_.erase(buffer_.begin());
    }

    // Add the new data point to the end of the buffer
    buffer_.push_back(newDataPoint);

    // Update largestOccupiedIndex_ if necessary
    if (buffer_.size() > largestOccupiedIndex_) {
        largestOccupiedIndex_ = buffer_.size() - 1;
    }
}

CpuUsageResult CpuUsageCalculator::calculateCpuUsagePercentForCore(int core) const {
    return calculateCpuUsagePercentForCore(core,0,largestOccupiedIndex_);
}

CpuUsageResult CpuUsageCalculator::calculateCpuUsagePercentForCore(int core, size_t index1, size_t index2) const {
    if (index1 >= bufferSize_ || index2 >= bufferSize_ || index1 == index2) {
        Printer& printer = Printer::getInstance(); // Get instance of Printer

        // Construct the buffer contents string
        std::string bufferContents;
        for (size_t i = 0; i < buffer_.size(); ++i) {
            const DataPoint& dp = buffer_[i];
            bufferContents += "Index " + std::to_string(i) + ": Jiffies: " + std::to_string(dp.jiffies) +
                              ", TotalUser: " + std::to_string(dp.totalUser) +
                              ", TotalUserLow: " + std::to_string(dp.totalUserLow) +
                              ", TotalSys: " + std::to_string(dp.totalSys) +
                              ", TotalIdle: " + std::to_string(dp.totalIdle) + "\n";
        }

        // Print warning with buffer contents
        printer.printWarning("Invalid indices. Variable Values: Buffer current size: " + std::to_string(buffer_.size()) +
                             "/" + std::to_string(bufferSize_) + ", Index1: " + std::to_string(index1) +
                             ", Index2: " + std::to_string(index2) + "\nBuffer Contents:\n" + bufferContents, __LINE__, __FILE__, -1);

        return { -1.0, 0 }; // Invalid indices
    }

    const DataPoint& dataPoint1 = buffer_[index1];
    const DataPoint& dataPoint2 = buffer_[index2];

    double notIdleDiff = (dataPoint2.totalUser - dataPoint1.totalUser) +
                         (dataPoint2.totalUserLow - dataPoint1.totalUserLow) +
                         (dataPoint2.totalSys - dataPoint1.totalSys);
    double idleDiff = dataPoint2.totalIdle - dataPoint1.totalIdle;

    double usagePercent = (notIdleDiff + idleDiff > 0) ? (notIdleDiff / (notIdleDiff + idleDiff)) * 100.0 : -1.0;

    // Calculate the jiffies passed
    unsigned long long jiffiesPassed = dataPoint2.jiffies - dataPoint1.jiffies;

    return { usagePercent, jiffiesPassed };
}

void CpuUsageCalculator::addDataPointForTotal(unsigned long long totalUser, unsigned long long totalUserLow,
                                              unsigned long long totalSys, unsigned long long totalIdle, unsigned long long jiffies) {
    addDataPointForCore(TOTAL_CPU_USAGE_INDEX, totalUser, totalUserLow, totalSys, totalIdle, jiffies);
}

CpuUsageResult CpuUsageCalculator::calculateCpuUsagePercentForTotal(size_t index1, size_t index2) const {
    return calculateCpuUsagePercentForCore(TOTAL_CPU_USAGE_INDEX, index1, index2);
}

CpuUsageResult CpuUsageCalculator::calculateCpuUsagePercentForTotal() const {
    return calculateCpuUsagePercentForCore(TOTAL_CPU_USAGE_INDEX, 0, largestOccupiedIndex_);
}

size_t CpuUsageCalculator::getWrappedIndex(size_t index) const {
    return index % bufferSize_;
}

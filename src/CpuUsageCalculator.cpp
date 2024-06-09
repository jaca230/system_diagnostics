#include "CpuUsageCalculator.h"
#include "ConfigManager.h"
#include "Printer.h"

// Initialize static variables
std::map<int, CpuUsageCalculator*> CpuUsageCalculator::coreInstances_;
std::mutex CpuUsageCalculator::mutex_;
const int CpuUsageCalculator::TOTAL_CPU_USAGE_INDEX = -1;

CpuUsageCalculator::CpuUsageCalculator() {
    // Initialize bufferSize_ with the value from ConfigManager
    bufferSize_ = ConfigManager::getInstance().getCpuUsageCalculatorBufferSize();
}

CpuUsageCalculator::~CpuUsageCalculator() {
    // Destructor implementation
}

CpuUsageCalculator& CpuUsageCalculator::getInstanceForCore(int core) {
    // Thread-safe creation of instances for each core and the total
    std::lock_guard<std::mutex> lock(mutex_);
    if (coreInstances_.find(core) == coreInstances_.end()) {
        coreInstances_[core] = new CpuUsageCalculator();
    }
    return *coreInstances_[core];
}

CpuUsageCalculator& CpuUsageCalculator::getInstanceForTotal() {
    // Thread-safe creation of instances for each core and the total
    std::lock_guard<std::mutex> lock(mutex_);
    if (coreInstances_.find(TOTAL_CPU_USAGE_INDEX) == coreInstances_.end()) {
        coreInstances_[TOTAL_CPU_USAGE_INDEX] = new CpuUsageCalculator();
    }
    return *coreInstances_[TOTAL_CPU_USAGE_INDEX];
}
void CpuUsageCalculator::addDataPointForTotal(unsigned long long totalUser, unsigned long long totalUserLow,
                                              unsigned long long totalSys, unsigned long long totalIdle) {
    addDataPointForCore(TOTAL_CPU_USAGE_INDEX,totalUser,totalUserLow,totalSys,totalIdle);
}
void CpuUsageCalculator::addDataPointForTotal(unsigned long long totalUser, unsigned long long totalUserLow,
                                              unsigned long long totalSys, unsigned long long totalIdle,
                                              std::chrono::system_clock::time_point timestamp) {
    addDataPointForCore(TOTAL_CPU_USAGE_INDEX,totalUser,totalUserLow,totalSys,totalIdle,timestamp);
}

CpuUsageResult CpuUsageCalculator::calculateCpuUsagePercentForTotal(size_t index1, size_t index2) const {
    return calculateCpuUsagePercentForCore(TOTAL_CPU_USAGE_INDEX,index1,index2);
}

CpuUsageResult CpuUsageCalculator::calculateCpuUsagePercentForTotal(std::chrono::milliseconds durationAgo) const {
    return calculateCpuUsagePercentForCore(TOTAL_CPU_USAGE_INDEX,durationAgo);
}

CpuUsageResult CpuUsageCalculator::calculateCpuUsagePercentForTotal(std::chrono::milliseconds durationAgo1, std::chrono::milliseconds durationAgo2) const {
    return calculateCpuUsagePercentForCore(TOTAL_CPU_USAGE_INDEX,durationAgo1,durationAgo2);
}

void CpuUsageCalculator::addDataPointForCore(int core, unsigned long long totalUser, unsigned long long totalUserLow,
                                              unsigned long long totalSys, unsigned long long totalIdle) {
    addDataPointForCore(core, totalUser, totalUserLow, totalSys, totalIdle, std::chrono::system_clock::now());
}

void CpuUsageCalculator::addDataPointForCore(int core, unsigned long long totalUser, unsigned long long totalUserLow,
                                              unsigned long long totalSys, unsigned long long totalIdle,
                                              std::chrono::system_clock::time_point timestamp) {
    DataPoint newDataPoint = {totalUser, totalUserLow, totalSys, totalIdle, timestamp};
    std::lock_guard<std::mutex> lock(mutex_);

    if (buffer_.size() >= bufferSize_) {
        buffer_.pop_front();
    }
    buffer_.push_back(newDataPoint);
}

CpuUsageResult CpuUsageCalculator::calculateCpuUsagePercentForCore(int core, size_t index1, size_t index2) const {
    Printer& printer = Printer::getInstance(); // Get instance of Printer
    if (index1 >= buffer_.size() || index2 >= buffer_.size() || index1 == index2) {
        printer.printWarning("Invalid indices. Variable Values: Buffer current size: " + std::to_string(buffer_.size()) +
                             "/" + std::to_string(bufferSize_) + ", Index1: " + std::to_string(index1) + ", Index2: " + std::to_string(index2), __LINE__, __FILE__, 1);
        return { -1.0, -1.0 }; // Invalid indices
    }
    if (index1 > index2) {
        std::swap(index1, index2);
    }
    const DataPoint& dataPoint1 = buffer_[index1];
    const DataPoint& dataPoint2 = buffer_[index2];
    printer.print("Data Point 1 at index " + std::to_string(index1) + ": Total User: " + std::to_string(dataPoint1.totalUser) +
                  ", Total User Low: " + std::to_string(dataPoint1.totalUserLow) +
                  ", Total Sys: " + std::to_string(dataPoint1.totalSys) +
                  ", Total Idle: " + std::to_string(dataPoint1.totalIdle), -1, "", 3);
    printer.print("Data Point 2 at index " + std::to_string(index2) + ": Total User: " + std::to_string(dataPoint2.totalUser) +
                  ", Total User Low: " + std::to_string(dataPoint2.totalUserLow) +
                  ", Total Sys: " + std::to_string(dataPoint2.totalSys) +
                  ", Total Idle: " + std::to_string(dataPoint2.totalIdle), -1, "", 3);
    double notIdleDiff = (dataPoint2.totalUser - dataPoint1.totalUser) + 
                         (dataPoint2.totalUserLow - dataPoint1.totalUserLow) + 
                         (dataPoint2.totalSys - dataPoint1.totalSys);
    double idleDiff = dataPoint2.totalIdle - dataPoint1.totalIdle;
    double usagePercent = -1.0;
    if (dataPoint2.totalUser - dataPoint1.totalUser >= 0 && dataPoint2.totalUserLow - dataPoint1.totalUserLow >= 0 
        && dataPoint2.totalSys - dataPoint1.totalSys >=0  && dataPoint2.totalIdle - dataPoint1.totalIdle >= 0) {
        //Percent used = Time not idle/total time
        usagePercent = notIdleDiff / (notIdleDiff + idleDiff) * 100.0;
    } else {
        printer.printWarning("Overflow detected in CPU usage calculation. Variable Values: Buffer current size: " + std::to_string(buffer_.size()) +
                             "/" + std::to_string(bufferSize_) + ", Index1: " + std::to_string(index1) + ", Index2: " + std::to_string(index2), __LINE__, __FILE__, 1);
    }
    std::chrono::duration<double> timeDiff = dataPoint2.timestamp - dataPoint1.timestamp;
    double timeDiffSeconds = timeDiff.count();
    return { usagePercent, timeDiffSeconds };
}

CpuUsageResult CpuUsageCalculator::calculateCpuUsagePercentForCore(int core, std::chrono::milliseconds durationAgo) const {
    size_t closestIndex = findClosestIndexForCore(core, durationAgo);
    size_t currentIndex = buffer_.size()-1;
    return calculateCpuUsagePercentForCore(core, closestIndex, currentIndex);
}

CpuUsageResult CpuUsageCalculator::calculateCpuUsagePercentForCore(int core, std::chrono::milliseconds durationAgo1, std::chrono::milliseconds durationAgo2) const {
    size_t closestIndex = findClosestIndexForCore(core, durationAgo1);
    size_t currentIndex = findClosestIndexForCore(core, durationAgo2);
    return calculateCpuUsagePercentForCore(core, closestIndex, currentIndex);
}

size_t CpuUsageCalculator::findClosestIndexForCore(int core, std::chrono::milliseconds durationAgo) const {
    auto targetTime = std::chrono::system_clock::now() - durationAgo;

    auto it = std::lower_bound(buffer_.begin(), buffer_.end(), targetTime,
        [](const DataPoint& dataPoint, const std::chrono::system_clock::time_point& time) {
            return dataPoint.timestamp < time;
        });

    // If no elements found or if the closest element is the first one
    if (it == buffer_.end() || it == buffer_.begin()) {
        return 0; // Return the first index
    }

    // Check which index is closer to the target time
    auto prevDiff = std::abs(std::chrono::duration_cast<std::chrono::milliseconds>(std::prev(it)->timestamp - targetTime).count());
    auto nextDiff = std::abs(std::chrono::duration_cast<std::chrono::milliseconds>(it->timestamp - targetTime).count());

    // Return the index closer to the target time
    return (prevDiff < nextDiff) ? std::distance(buffer_.begin(), std::prev(it)) : std::distance(buffer_.begin(), it);
}

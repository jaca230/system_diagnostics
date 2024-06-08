#include "CpuUsageCalculator.h"
#include "Printer.h"
#include <string>

CpuUsageCalculator* CpuUsageCalculator::instance_ = nullptr;
std::mutex CpuUsageCalculator::mutex_;

CpuUsageCalculator& CpuUsageCalculator::getInstance() {
    // Thread-safe singleton creation
    std::lock_guard<std::mutex> lock(mutex_);
    if (!instance_) {
        instance_ = new CpuUsageCalculator();
    }
    return *instance_;
}

CpuUsageCalculator::CpuUsageCalculator() {}

CpuUsageCalculator::~CpuUsageCalculator() {
    // Destructor implementation
}

void CpuUsageCalculator::addDataPoint(unsigned long long totalUser, unsigned long long totalUserLow,
                                      unsigned long long totalSys, unsigned long long totalIdle) {
    // Add a new data point to the buffer with the current timestamp
    DataPoint newDataPoint = {totalUser, totalUserLow, totalSys, totalIdle, std::chrono::system_clock::now()};
    buffer_.push_back(newDataPoint);
}

void CpuUsageCalculator::addDataPoint(unsigned long long totalUser, unsigned long long totalUserLow,
                                      unsigned long long totalSys, unsigned long long totalIdle,
                                      std::chrono::system_clock::time_point timestamp) {
    // Add a new data point to the buffer with the specified timestamp
    DataPoint newDataPoint = {totalUser, totalUserLow, totalSys, totalIdle, timestamp};
    buffer_.push_back(newDataPoint);
}

CpuUsageResult CpuUsageCalculator::calculateCpuUsagePercent(size_t index1, size_t index2) const {
    Printer& printer = Printer::getInstance(); // Get instance of Printer

    if (index1 >= buffer_.size() || index2 >= buffer_.size() || index1 >= index2) {
        printer.printWarning("Invalid indices or order. Variable Values: Buffer size: " + std::to_string(buffer_.size()) +
              ", Index1: " + std::to_string(index1) + ", Index2: " + std::to_string(index2), -1, "", 2);
        return { -1.0, -1.0 }; // Invalid indices or order
    }

    const DataPoint& dataPoint1 = buffer_[index1];
    const DataPoint& dataPoint2 = buffer_[index2];

    double totalDiff = (dataPoint2.totalUser - dataPoint1.totalUser) + 
                       (dataPoint2.totalUserLow - dataPoint1.totalUserLow) + 
                       (dataPoint2.totalSys - dataPoint1.totalSys);

    double idleDiff = dataPoint2.totalIdle - dataPoint1.totalIdle;

    double usagePercent = -1.0;

    if (totalDiff >= 0 && idleDiff >= 0 && totalDiff >= idleDiff) {
        usagePercent = (totalDiff - idleDiff) / totalDiff * 100.0;
    } else {
        printer.printWarning("Overflow detected in CPU usage calculation.", -1, "", 2);
    }

    std::chrono::duration<double> timeDiff = dataPoint2.timestamp - dataPoint1.timestamp;
    double timeDiffSeconds = timeDiff.count();

    return { usagePercent, timeDiffSeconds };
}

CpuUsageResult CpuUsageCalculator::calculateCpuUsagePercentFromDuration(std::chrono::seconds durationAgo) const {
    size_t closestIndex = findClosestIndex(durationAgo);
    size_t currentIndex = buffer_.size() - 1; // Last index (current time)

    return calculateCpuUsagePercent(closestIndex, currentIndex);
}

size_t CpuUsageCalculator::findClosestIndex(std::chrono::seconds durationAgo) const {
    auto targetTime = std::chrono::system_clock::now() - durationAgo;

    auto it = std::lower_bound(buffer_.begin(), buffer_.end(), targetTime,
        [](const DataPoint& dataPoint, const std::chrono::system_clock::time_point& time) {
            return dataPoint.timestamp < time;
        });

    if (it == buffer_.end()) {
        return buffer_.size() - 1; // If no elements found, return the last index
    }

    return std::distance(buffer_.begin(), it);
}

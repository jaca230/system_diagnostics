#ifndef CPU_USAGE_CALCULATOR_H
#define CPU_USAGE_CALCULATOR_H

#include <vector>
#include <chrono>
#include <mutex>

struct DataPoint {
    unsigned long long totalUser;
    unsigned long long totalUserLow;
    unsigned long long totalSys;
    unsigned long long totalIdle;
    std::chrono::system_clock::time_point timestamp;
};

struct CpuUsageResult {
    double usagePercent;
    double timeDiffSeconds;
};

class CpuUsageCalculator {
public:
    static CpuUsageCalculator& getInstance();
    void addDataPoint(unsigned long long totalUser, unsigned long long totalUserLow,
                      unsigned long long totalSys, unsigned long long totalIdle);
    void addDataPoint(unsigned long long totalUser, unsigned long long totalUserLow,
                      unsigned long long totalSys, unsigned long long totalIdle,
                      std::chrono::system_clock::time_point timestamp);
    CpuUsageResult calculateCpuUsagePercent(size_t index1, size_t index2) const;
    CpuUsageResult calculateCpuUsagePercentFromDuration(std::chrono::seconds durationAgo) const;

private:
    CpuUsageCalculator();
    ~CpuUsageCalculator();

    size_t findClosestIndex(std::chrono::seconds durationAgo) const;

    std::vector<DataPoint> buffer_;
    static CpuUsageCalculator* instance_;
    static std::mutex mutex_;
};

#endif // CPU_USAGE_CALCULATOR_H

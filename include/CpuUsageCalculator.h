#ifndef CPU_USAGE_CALCULATOR_H
#define CPU_USAGE_CALCULATOR_H

#include <deque>
#include <chrono>
#include <mutex>
#include <map>

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
    static CpuUsageCalculator& getInstanceForCore(int core);
    static CpuUsageCalculator& getInstanceForTotal();
    // Public constant for the total CPU index (generally -1)
    static const int TOTAL_CPU_USAGE_INDEX;

    void addDataPointForCore(int core, unsigned long long totalUser, unsigned long long totalUserLow,
                              unsigned long long totalSys, unsigned long long totalIdle);
    void addDataPointForCore(int core, unsigned long long totalUser, unsigned long long totalUserLow,
                              unsigned long long totalSys, unsigned long long totalIdle,
                              std::chrono::system_clock::time_point timestamp);
    CpuUsageResult calculateCpuUsagePercentForCore(int core, size_t index1, size_t index2) const;
    CpuUsageResult calculateCpuUsagePercentForCore(int core, std::chrono::milliseconds durationAgo) const;
    CpuUsageResult calculateCpuUsagePercentForCore(int core, std::chrono::milliseconds durationAgo1, std::chrono::milliseconds durationAgo2) const;
    void addDataPointForTotal(unsigned long long totalUser, unsigned long long totalUserLow,
                              unsigned long long totalSys, unsigned long long totalIdle);
    void addDataPointForTotal(unsigned long long totalUser, unsigned long long totalUserLow,
                              unsigned long long totalSys, unsigned long long totalIdle,
                              std::chrono::system_clock::time_point timestamp);
    CpuUsageResult calculateCpuUsagePercentForTotal(size_t index1, size_t index2) const;
    CpuUsageResult calculateCpuUsagePercentForTotal(std::chrono::milliseconds durationAgo) const;
    CpuUsageResult calculateCpuUsagePercentForTotal(std::chrono::milliseconds durationAgo1, std::chrono::milliseconds durationAgo2) const;

private:
    CpuUsageCalculator();
    ~CpuUsageCalculator();

    size_t findClosestIndexForCore(int core, std::chrono::milliseconds durationAgo) const;

    static std::map<int, CpuUsageCalculator*> coreInstances_;
    static std::mutex mutex_;
    size_t bufferSize_;
    std::deque<DataPoint> buffer_;
};

#endif // CPU_USAGE_CALCULATOR_H

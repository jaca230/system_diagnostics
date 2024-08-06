#ifndef CPU_USAGE_CALCULATOR_H
#define CPU_USAGE_CALCULATOR_H

#include <deque>
#include <mutex>
#include <vector>
#include <limits> // For std::numeric_limits
#include <map>

struct DataPoint {
    unsigned long long totalUser;
    unsigned long long totalUserLow;
    unsigned long long totalSys;
    unsigned long long totalIdle;
    unsigned long long jiffies;
};

struct CpuUsageResult {
    double usagePercent;
    unsigned long long jiffiesPassed;
};

class CpuUsageCalculator {
public:
    static CpuUsageCalculator& getInstanceForCore(int core);
    static CpuUsageCalculator& getInstanceForTotal();
    static const int TOTAL_CPU_USAGE_INDEX;

    void addDataPointForCore(int core, unsigned long long totalUser, unsigned long long totalUserLow,
                              unsigned long long totalSys, unsigned long long totalIdle, unsigned long long jiffies);
    CpuUsageResult calculateCpuUsagePercentForCore(int core, size_t index1, size_t index2) const;
    CpuUsageResult calculateCpuUsagePercentForCore(int core) const;
    void addDataPointForTotal(unsigned long long totalUser, unsigned long long totalUserLow,
                              unsigned long long totalSys, unsigned long long totalIdle, unsigned long long jiffies);
    CpuUsageResult calculateCpuUsagePercentForTotal(size_t index1, size_t index2) const;
    CpuUsageResult calculateCpuUsagePercentForTotal() const;

private:
    CpuUsageCalculator();
    ~CpuUsageCalculator();

    static std::map<int, CpuUsageCalculator*> coreInstances_;
    static std::mutex mutex_;
    size_t bufferSize_;
    std::vector<DataPoint> buffer_;
    size_t currentIndex_;
    size_t largestOccupiedIndex_;

    size_t getWrappedIndex(size_t index) const;
};

#endif // CPU_USAGE_CALCULATOR_H

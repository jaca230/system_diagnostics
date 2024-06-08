#ifndef CPUINFO_H
#define CPUINFO_H

#include <sys/sysinfo.h>
#include <unistd.h>
#include <cstdio>

class CpuInfo {
public:
  // Constructor
  CpuInfo();

  // Function to gather and store CPU information
  void GetCpuInfo();

  // Functions to access gathered information
  long GetTotalRamMB() const;
  long GetFreeRamMB() const;
  double GetLoadAvg1Min() const;
  double GetLoadAvg5Min() const;
  double GetLoadAvg15Min() const;
  double GetCpuUsage() const;

private:
  // Member variables to store gathered information
  long uptime_;
  unsigned long totalram_;
  double load_avg_[3];  // Store load averages for 1, 5, and 15 minutes

  // CPU usage member variables
  mutable unsigned long long lastTotalUser_;
  mutable unsigned long long lastTotalUserLow_;
  mutable unsigned long long lastTotalSys_;
  mutable unsigned long long lastTotalIdle_;

  // Private method to initialize CPU usage
  void InitCpuUsage() const;
};

#endif // CPUINFO_H

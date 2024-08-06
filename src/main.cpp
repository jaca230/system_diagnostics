#include <iostream>
#include <thread>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include "ConfigManager.h"
#include "Printer.h"
#include "SystemInfo.h"

void printHelp() {
    std::cout << "Usage: ./your_program [options]\n"
              << "Options:\n"
              << "  -i, --iterations <number>   Number of iterations (default: 10)\n"
              << "  -d, --delay <milliseconds>  Delay between iterations in milliseconds (default: 100)\n"
              << "  -h, --help                  Show this help message\n";
}

void parseCommandLineArgs(int argc, char* argv[], int& iterations, int& delayMilliseconds) {
    iterations = 10;
    delayMilliseconds = 100;

    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "-i") == 0 || std::strcmp(argv[i], "--iterations") == 0) {
            if (i + 1 < argc) {
                iterations = std::atoi(argv[++i]);
            } else {
                std::cerr << "Error: --iterations option requires a number.\n";
                exit(1);
            }
        } else if (std::strcmp(argv[i], "-d") == 0 || std::strcmp(argv[i], "--delay") == 0) {
            if (i + 1 < argc) {
                delayMilliseconds = std::atoi(argv[++i]);
            } else {
                std::cerr << "Error: --delay option requires a number.\n";
                exit(1);
            }
        } else if (std::strcmp(argv[i], "-h") == 0 || std::strcmp(argv[i], "--help") == 0) {
            printHelp();
            exit(0);
        } else {
            std::cerr << "Error: Unknown option " << argv[i] << "\n";
            printHelp();
            exit(1);
        }
    }
}

int main(int argc, char* argv[]) {
    int iterations;
    int delayMilliseconds;
    
    parseCommandLineArgs(argc, argv, iterations, delayMilliseconds);

    // Get instance of Printer
    Printer& printer = Printer::getInstance(); 

    // Create and start SystemInfo instance
    SystemInfo& systemInfo = SystemInfo::getInstance();
    systemInfo.startPeriodicUpdates();

    
    for (int i = 0; i < iterations; ++i) {
        printer.print("-------------------------------");
        printer.print("Iteration #" + std::to_string(i+1));
        printer.print("-------------------------------");

        // Collect system information
        SystemInfoData data = systemInfo.collectSystemInfo();

        // Print system information
        printer.print("Total RAM: " + std::to_string(data.total_ram) + " B");
        printer.print("Free RAM: " + std::to_string(data.free_ram) + " B");
        printer.print("Total RAM (MB): " + std::to_string(data.total_ram_MB) + " MB");
        printer.print("Free RAM (MB): " + std::to_string(data.free_ram_MB) + " MB");
        printer.print("Total CPU Usage: " + std::to_string(data.cpu_usage_percent) + "%");
        printer.print("Time step for CPU Usage: " + std::to_string(data.cpu_real_time_step) + "s");
        printer.print("Number of CPU Processors: " + std::to_string(data.cpu_num_processors));
        
        for (int core = 0; core < data.cpu_num_processors; ++core) {
            printer.print("CPU Core " + std::to_string(core) + " Usage: " + std::to_string(data.cpu_usage_percent_per_core[core]) + "%");
            printer.print("CPU Core " + std::to_string(core) + " Time step: " + std::to_string(data.cpu_real_time_step_per_core[core]) + "s");
        }

        // Print load averages
        printer.print("Load Average (1 min, 5 min, 15 min): " +
                      std::to_string(data.load_avg_1min) + " " +
                      std::to_string(data.load_avg_5min) + " " +
                      std::to_string(data.load_avg_15min));

        // Package system information for MIDAS
        std::vector<double> systemInfoData = systemInfo.packageSystemInfoForMIDAS();
        printer.print("System Info for MIDAS: ");
        for (size_t i = 0; i < systemInfoData.size(); ++i) {
            printer.print("systemInfoData[" + std::to_string(i) + "] = " + std::to_string(systemInfoData[i]));
        }

        // Delay before the next iteration
        std::this_thread::sleep_for(std::chrono::milliseconds(delayMilliseconds));
    }


    return 0;
}

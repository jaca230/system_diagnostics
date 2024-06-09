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
    // Default values
    iterations = 10;
    delayMilliseconds = 100;

    // Parse command-line arguments
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

    // Create an instance of ConfigManager to read the configuration file
    ConfigManager& configManager = ConfigManager::getInstance();
    // Get instance of Printer
    Printer& printer = Printer::getInstance(); 
    // Create an instance of SystemInfo
    SystemInfo& SystemInfo = SystemInfo::getInstance();

    for (int i = 0; i < iterations; ++i) {
        printer.print("-------------------------------");
        printer.print("Iteration #" + std::to_string(i+1));
        printer.print("-------------------------------");
        // Update CPU information
        SystemInfo.updateSystemInfo();

        // Get total RAM, free RAM, and CPU usage percentage
        long total_ram = SystemInfo.getTotalRam();
        long free_ram = SystemInfo.getFreeRam();
        long total_ram_MB = SystemInfo.getTotalRamMB();
        long free_ram_MB = SystemInfo.getFreeRamMB();
        double cpu_usage_percent = SystemInfo.getCpuUsage();
        int cpu_num_processors = SystemInfo.getNumCores();
        double cpu_real_time_step = SystemInfo.getTimeStep();

        // Print total RAM, free RAM, and CPU usage
        printer.print("Total RAM: " + std::to_string(total_ram) + " B");
        printer.print("Free RAM: " + std::to_string(free_ram) + " B");
        printer.print("Total RAM (MB): " + std::to_string(total_ram_MB) + " MB");
        printer.print("Free RAM (MB): " + std::to_string(free_ram_MB) + " MB");
        printer.print("Total CPU Usage: " + std::to_string(cpu_usage_percent) + "%");
        printer.print("Time step for CPU Usage: " + std::to_string(cpu_real_time_step) + "s");
        printer.print("Number of CPU Proccessors: " + std::to_string(cpu_num_processors));
        for (int core = 0; core < cpu_num_processors; ++core) {
            double cpu_usage_percent_core = SystemInfo.getCpuUsageForCore(core);
            double cpu_real_time_step_core = SystemInfo.getTimeStepForCore(core);
            printer.print("CPU Core " + std::to_string(core) + " Usage: " + std::to_string(cpu_usage_percent_core) + "%");
            printer.print("CPU Core " + std::to_string(core) + " Time step: " + std::to_string(cpu_real_time_step_core) + "s");
        }

        // Print load averages for 1 min, 5 min, and 15 min
        std::string load_avg_msg = "Load Average (1 min, 5 min, 15 min): ";
        load_avg_msg += std::to_string(SystemInfo.getLoadAvg1Min()) + " ";
        load_avg_msg += std::to_string(SystemInfo.getLoadAvg5Min()) + " ";
        load_avg_msg += std::to_string(SystemInfo.getLoadAvg15Min());
        printer.print(load_avg_msg);

        // Delay before the next iteration
        std::this_thread::sleep_for(std::chrono::milliseconds(delayMilliseconds));
    }

    return 0;
}

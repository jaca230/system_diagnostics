# System Diagnostics

## Overview

This repository provides tools for calculating CPU usage and other system diagnostics on Linux systems. This tool is built for efficient monitoring and provide some configuration options.

---

## Installation Instructions

### Option 1: Using the Build Script

1. Clone the repository:
    ```bash
    git clone https://github.com/jaca230/system_diagnostics.git
    cd system_diagnostics
    ```

2. Run the build script:
    ```bash
    ./build.sh
    ```

### Option 2: Manual Build

1. Clone the repository:
    ```bash
    git clone https://github.com/jaca230/system_diagnostics.git
    cd system_diagnostics
    ```

2. Create a build directory and navigate to it:
    ```bash
    mkdir build
    cd build
    ```

3. Run CMake to configure the build:
    ```bash
    cmake ..
    ```

4. Build and install the project:
    ```bash
    make install -j$(nproc)
    ```

### Executable and Library Outputs

- The executables will be located in the `bin/` directory.
- The libraries will be located in the `lib/` directory.

---

## Usage

To use the system diagnostics tools, navigate to the `bin/` directory and run the desired executable. For example:
```bash
cd bin
./system_diagnostics --help
./system_diagnostics
```

---

## Configuration

The behavior of the system diagnostics tool can be customized using a configuration file written in JSON format. Below is a description of each configuration section and its options.

### Config File Sections

#### **`debug`**
- **`verbosity`**:
  - **Description**: Controls the level of debug output. Higher values produce more detailed logs.
  - **Example**: 
    - `0`: Minimal output (default).
    - `1`: Includes additional output and warnings.
    - `2`: Includes even more output and warnings, etc.

#### **`printer`**
- **`print_line_number`**:
  - **Description**: Whether to include the source code line number in log messages.
  - **Example**: `true` (enabled) or `false` (disabled).
  
- **`print_current_time`**:
  - **Description**: Whether to include a timestamp in log messages.
  - **Example**: `true` (enabled) or `false` (disabled).

- **`prefix`** and **`suffix`**:
  - **Description**: Strings added before and after log messages, respectively.
  - **Example**: `"system_monitor"` for `prefix`.

- **`info_color`, `warning_color`, `error_color`**:
  - **Description**: Define the colors for log messages based on their severity.
  - **Options**: `"white"`, `"yellow"`, `"red"`, `"green"`, `"black"`, `"blue"`, `"magenta"`, or `"cyan"`

#### **`system_info`**
- **`update_period_jiffies`**:
  - **Description**: Determines how frequently (in jiffies) CPU usage data is polled and recorded.
  - **Note**: A jiffy is a system-dependent unit of time (often 10 ms). You can find your system’s jiffy rate using `getconf CLK_TCK`.
  - **Example**: `20` jiffies (200 ms at 10 ms/jiffy).

- **`average_period_jiffies`**:
  - **Description**: Specifies the time period (in jiffies) over which the average CPU usage is calculated.
  - **Example**: `100` jiffies (1 second at 10 ms/jiffy).

---

### Example Config File

Located in `config/config.json`:

```json
{
    "debug": {
        "verbosity": 0
    },
    "printer": {
        "print_line_number": true,
        "print_current_time": true,
        "prefix": "system_monitor",
        "suffix": "",
        "info_color": "white",
        "warning_color": "yellow",
        "error_color": "red"
    },
    "system_info": {
        "NOTE": "A jiffy is a unit defined by your system, usually 10 ms. See `getconf CLK_TCK` for the rate in Hz.",
        "update_period_jiffies": 20,
        "average_period_jiffies": 100
    }
}
```

---

## Notes

- **Accuracy**: The program may produce inaccurate results if:
  1. The CPU usage is polled too frequently.
  2. The total CPU usage is small.

This is sort of an "uncertainty principle." If the CPU usage is too small in the polled timeframe, the act of measuring it changes the result. For example, if there CPU is at 0% usage in total, and we measure that, it takes some CPU usage to measure that. So measuring each core's usage after that may show a non-zero value.

Furthermore there are small innaccuracies due to rounding errors.


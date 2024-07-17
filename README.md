# System Diagnostics

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
    git clone https://github.com/yourusername/system_diagnostics.git
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

## Usage

To use the system diagnostics tools, navigate to the `bin/` directory and run the desired executable. For example:
```bash
cd bin
./system_diagnostics --help
./system_diagnostics

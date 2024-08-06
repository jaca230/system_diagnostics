#!/bin/bash

# Get the directory of the script
SOURCE="${BASH_SOURCE[0]}"
while [ -L "$SOURCE" ]; do
    DIR=$( cd -P "$( dirname "$SOURCE" )" >/dev/null 2>&1 && pwd )
    SOURCE=$(readlink "$SOURCE")
    [[ $SOURCE != /* ]] && SOURCE="$DIR/$SOURCE"
done
script_directory=$( cd -P "$( dirname "$SOURCE" )" >/dev/null 2>&1 && pwd )

# Build directory path
build_directory=$(realpath "$script_directory/../build")

# Navigate into build directory
cd "$build_directory"

# Try cmake3 first, then fallback to cmake if cmake3 is not found
if command -v cmake3 &> /dev/null; then
    cmake3 ..
else
    cmake ..
fi

# Run make install with parallel jobs based on the number of processors
make install -j$(nproc)

# Return to original directory
cd "$script_directory"
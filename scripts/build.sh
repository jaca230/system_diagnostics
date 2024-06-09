#!/bin/bash

# Function to display help
display_help() {
    echo "Usage: $0 [options]"
    echo
    echo "Options:"
    echo "  --overwrite   Overwrite the build directory if it exists"
    echo "  --help        Display this help message"
    exit 0
}

# Parse command-line arguments
overwrite=false
for arg in "$@"; do
    case $arg in
        --overwrite)
        overwrite=true
        shift
        ;;
        --help)
        display_help
        ;;
        *)
        echo "Unknown option: $arg"
        display_help
        ;;
    esac
done

# Get the directory of the script
SOURCE="${BASH_SOURCE[0]}"
while [ -L "$SOURCE" ]; do
    DIR=$( cd -P "$( dirname "$SOURCE" )" >/dev/null 2>&1 && pwd )
    SOURCE=$(readlink "$SOURCE")
    [[ $SOURCE != /* ]] && SOURCE="$DIR/$SOURCE"
done
script_directory=$( cd -P "$( dirname "$SOURCE" )" >/dev/null 2>&1 && pwd )

# Source utilities.sh from lib directory
source "$script_directory/lib/utilities.sh"

# Get the calling directory
user_dir=$(get_calling_directory)

# Source setup_env.sh to ensure environment is set up
source "$script_directory/setup_env.sh"

# Build directory path
build_directory=$(realpath "$script_directory/../build")

# Remove the build directory if --overwrite flag is set
if $overwrite && [ -d "$build_directory" ]; then
    rm -rf "$build_directory"
fi

# Create build directory if it doesn't exist
mkdir -p "$build_directory"

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
cd "$user_dir"

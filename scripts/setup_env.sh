#!/bin/bash

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

# Ensure the script is sourced
if ! is_sourced_script; then
    echo "This script must be sourced. Please run with 'source setup_env.sh' or '. setup_env.sh'."
    return 1
fi

# Define the bin directory relative to the script directory
bin_directory="$script_directory/../bin"

# Add the bin directory to PATH if it's not already included
if ! is_in_path "$bin_directory"; then
    export PATH="$bin_directory:$PATH"
    echo "Added $bin_directory to PATH."
else
    echo "$bin_directory is already in PATH."
fi
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
bin_directory=$(realpath "$script_directory/../bin")

# Add the bin directory to PATH if it's not already included
if ! is_in_path "$bin_directory"; then
    export PATH="$bin_directory:$PATH"
    echo "Added $bin_directory to PATH."
else
    echo "$bin_directory is already in PATH."
fi

# Source the environment configuration file
config_file="$script_directory/config/env_config.sh"
if [[ -f "$config_file" ]]; then
    source "$config_file"
    echo "Sourced environment configuration file: $config_file"
else
    echo "Configuration file not found: $config_file"
    return 1
fi

# Convert the SYSTEM_MONITOR_CONFIG_FILE_PATH to an absolute path
if [[ -n "$SYSTEM_MONITOR_CONFIG_FILE_PATH" ]]; then
    export SYSTEM_MONITOR_CONFIG_FILE_PATH=$(realpath "$SYSTEM_MONITOR_CONFIG_FILE_PATH")
    echo "SYSTEM_MONITOR_CONFIG_FILE_PATH set to: $SYSTEM_MONITOR_CONFIG_FILE_PATH"
else
    echo "SYSTEM_MONITOR_CONFIG_FILE_PATH is not set in the configuration file."
    return 1
fi
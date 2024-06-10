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

# Default value for quiet flag
quiet=0

# Function to print messages based on the quiet flag
log() {
    if [ $quiet -eq 0 ]; then
        echo "$1"
    fi
}

# Parse arguments
while [[ $# -gt 0 ]]; do
    case "$1" in
        -q|--quiet)
            quiet=1
            shift
            ;;
        *)
            shift
            ;;
    esac
done

# Ensure the script is sourced
if ! is_sourced_script; then
    log "This script must be sourced. Please run with 'source setup_env.sh' or '. setup_env.sh'."
    return 1
fi

# Define the bin directory relative to the script directory
bin_directory=$(realpath "$script_directory/../bin")

# Add the bin directory to PATH if it's not already included
if ! is_in_path "$bin_directory"; then
    export PATH="$bin_directory:$PATH"
    log "Added $bin_directory to PATH."
else
    log "$bin_directory is already in PATH."
fi

# Source the environment configuration file
config_file="$script_directory/config/env_config.sh"
if [[ -f "$config_file" ]]; then
    source "$config_file"
    log "Sourced environment configuration file: $config_file"
else
    log "Configuration file not found: $config_file"
    return 1
fi

# Convert the SYSTEM_MONITOR_CONFIG_FILE_PATH to an absolute path
if [[ -n "$SYSTEM_MONITOR_CONFIG_FILE_PATH" ]]; then
    export SYSTEM_MONITOR_CONFIG_FILE_PATH=$(realpath "$SYSTEM_MONITOR_CONFIG_FILE_PATH")
    log "SYSTEM_MONITOR_CONFIG_FILE_PATH set to: $SYSTEM_MONITOR_CONFIG_FILE_PATH"
else
    log "SYSTEM_MONITOR_CONFIG_FILE_PATH is not set in the configuration file."
    return 1
fi

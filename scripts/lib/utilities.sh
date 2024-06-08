#!/bin/bash

# Function to check if a directory is in PATH
is_in_path() {
    local dir="$1"
    if [[ ":$PATH:" == *":$dir:"* ]]; then
        return 0
    else
        return 1
    fi
}

# Function to check if a script is sourced
is_sourced_script() {
    if [[ "${BASH_SOURCE[0]}" != "${0}" ]]; then
        return 0
    else
        return 1
    fi
}

# Function to get the directory from which the script was called
get_calling_directory() {
    echo "$PWD"
}
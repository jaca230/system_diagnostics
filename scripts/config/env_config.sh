# config/env_config.sh
# Environment variables for system diagnostics
# Get the directory of the script
SOURCE="${BASH_SOURCE[0]}"
while [ -L "$SOURCE" ]; do
    DIR=$( cd -P "$( dirname "$SOURCE" )" >/dev/null 2>&1 && pwd )
    SOURCE=$(readlink "$SOURCE")
    [[ $SOURCE != /* ]] && SOURCE="$DIR/$SOURCE"
done
config_script_directory=$( cd -P "$( dirname "$SOURCE" )" >/dev/null 2>&1 && pwd )

# Here you can change this to point to wherever you want to place your config file.
# By default paths are set relative to this project's home directory
export SYSTEM_MONITOR_CONFIG_FILE_PATH=$(realpath $config_script_directory/../../config/config.json)


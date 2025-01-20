#!/bin/bash

# Ensure the script exits on any error
set -e

# Check if at least one argument is passed
if [ "$#" -lt 1 ]; then
    echo "Usage: $0 <python_args>"
    echo "Example: $0 data.csv --xrange 0to10 --yrange 0to5 --max"
    exit 1
fi

# Define paths
SHARED_LIB_DIR="./shared_libraries"
SHARED_LIB_NAME="matrix_lib.so"
SHARED_LIB_SOURCE="./data_preperation/matrix_lib.c"
PYTHON_SCRIPT="./cli_parser.py"

# Step 1: Compile the shared library
gcc -shared -fPIC -o "$SHARED_LIB_DIR/$SHARED_LIB_NAME" "$SHARED_LIB_SOURCE" -lpthread

# Step 2: Run the Python script with the passed arguments
python3 "$PYTHON_SCRIPT" "$@"

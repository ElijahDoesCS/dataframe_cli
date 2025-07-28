#!/bin/bash

# Exit on error
set -e

# Check for at least one argument
if [ "$#" -lt 1 ]; then
    echo "Usage: $0 <python_args> [--memcheck]"
    echo "Example: $0 ./dataframes/example2.csv --xrange 0to10 --yrange 0to5 --max [--memcheck]"
    exit 1
fi

# Define paths
SHARED_LIB_DIR="./shared_libraries"
SHARED_LIB_NAME="libmatrix_lib.so"
STATISTICAL_OPS_SOURCE="./data_preperation/arithmetic_lib/statistical_ops/statistical_ops.c"
MATRIX_LIB_SOURCE="./data_preperation/cli_ops/matrix_lib.c"
FAT_DATA_SOURCE="./data_preperation/arithmetic_lib/fat_data/fat_data.c"
MARSHALLER_SOURCE="./data_preperation/cli_ops/marshaller/marshaller.c"
MERGE_SORT_SOURCE="./data_preperation/arithmetic_lib/sorting/merge/merge.c"
HASHMAP_SOURCE="./data_preperation/arithmetic_lib/hashmap/hashmap.c"
PYTHON_SCRIPT="./cli_parser.py"
HOOK_C="./dev_functionality/valgrind/valgrind_driver.c"
HOOK_EXEC="./dev_functionality/valgrind/valgrind_runner"
valgrind_LOG_DIR="./memory_logs"
valgrind_LOG_PREFIX="$valgrind_LOG_DIR/valgrind_log"
K_WAY_MERGE_SOURCE="./data_preperation/arithmetic_lib/sorting/k_way/k_way.c"


# Create necessary folders
mkdir -p "$SHARED_LIB_DIR"
mkdir -p "$valgrind_LOG_DIR"

# Parse arguments and remove --memcheck
MEMCHECK=false
RERUN=false
ARGS=()
OPERATIONS=4  # Default: mean
THREAD_COUNT=1  # Default: 1

for arg in "$@"; do
    if [ "$arg" == "--memcheck" ]; then
        MEMCHECK=true
    elif [ "$arg" == "--rerun" ]; then
        RERUN=true
    elif [[ "$arg" == --operations=* ]]; then # Bit of a quark, parses a literal number for memcheck's operations
        OPERATIONS="${arg#--operations=}"
    elif [[ "$arg" == --thread-count=* ]]; then
        THREAD_COUNT="${arg#--thread-count=}"
    else
        ARGS+=("$arg")
    fi
done

# --rerun is only allowed with --memcheck
if [ "$RERUN" = true ] && [ "$MEMCHECK" = false ]; then
    echo "[❌] Error: '--rerun' must be used with '--memcheck'"
    exit 1
fi

# Memory check mode with Valgrind
if [ "$MEMCHECK" = true ]; then
    echo "[🔍] Running memory check with Valgrind..."

    # Skip input generation if rerunning
    if [ "$RERUN" = false ]; then
        echo "[📂] Generating new input command file..."
        python3 ./dev_functionality/file_generation/command_generation.py
    else
        echo "[♻️] Reusing existing input command file..."
    fi

    # Compile shared lib WITHOUT valgrind (no sanitizer needed for Valgrind)
    gcc -shared -fPIC -g -O2 -o "$SHARED_LIB_DIR/$SHARED_LIB_NAME" \
        "$MATRIX_LIB_SOURCE" "$FAT_DATA_SOURCE" "$MARSHALLER_SOURCE" "$STATISTICAL_OPS_SOURCE" "$MERGE_SORT_SOURCE" "$K_WAY_MERGE_SOURCE" "$HASHMAP_SOURCE" -lpthread

    # Compile the hook runner (C entry point)
    gcc -g -O2 -o "$HOOK_EXEC" "$HOOK_C" -L"$SHARED_LIB_DIR" -lmatrix_lib

    # Ensure hook can find shared lib at runtime
    export LD_LIBRARY_PATH="$SHARED_LIB_DIR:$LD_LIBRARY_PATH"

    # Path to the commands file
    COMMANDS_FILE="./dev_functionality/valgrind/example_commands/commands.txt"

    # Generate timestamped log directory
    TIMESTAMP=$(date +%Y%m%d_%H%M%S)
    LOG_DIR="./memory_logs/run_$TIMESTAMP"
    mkdir -p "$LOG_DIR"

    echo "[📄] Reading command lines from: $COMMANDS_FILE"

    # Loop through each line of generated commands and pass to Valgrind hook runner
    while IFS= read -r line || [[ -n "$line" ]]; do
        echo
        echo "[▶️] : $line"

        # Extract the base CSV filename and arguments
        IFS=' ' read -r csv_file y0 y1 x0 x1 <<< "$line"
        base_csv=$(basename "$csv_file" .csv)

        # Safe filename: replace any spaces or slashes
        file_name="${base_csv}_${y0}_${y1}_${x0}_${x1}"
        file_name=$(echo "$file_name" | tr -s '/ ' '_')
        log_file="$LOG_DIR/${file_name}.txt"

        # Run valgrind in a subshell to prevent it from exiting the whole script
        (
            valgrind \
                --leak-check=full \
                --show-leak-kinds=all \
                --track-origins=yes \
                --log-file="$log_file" \
                "$HOOK_EXEC" "$csv_file" "$y0" "$y1" "$x0" "$x1" "$OPERATIONS" "$THREAD_COUNT" # Append ops & thread count
        ) || echo "[⚠️] Valgrind exited with failure: $file_name" 

    done < "$COMMANDS_FILE"

# Normal mode (no memcheck)
else
    echo "[🚀] Running in standard mode..."

    # Compile shared lib without Valgrind
    gcc -shared -fPIC -g -O2 -o "$SHARED_LIB_DIR/$SHARED_LIB_NAME" \
        "$MATRIX_LIB_SOURCE" "$FAT_DATA_SOURCE" "$MARSHALLER_SOURCE" "$STATISTICAL_OPS_SOURCE" "$MERGE_SORT_SOURCE" "$K_WAY_MERGE_SOURCE" "$HASHMAP_SOURCE" -lpthread

    # Run Python script as usual, passing operations and thread count
    python3 "$PYTHON_SCRIPT" "${ARGS[@]}"
fi






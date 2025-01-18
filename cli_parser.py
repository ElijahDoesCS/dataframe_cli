# Perform threaded statistical operation on CSV files using the C pthreads library
# Use AI to optimize thread count in correlation with file size

import argparse
import os
import ctypes
import numpy as np
import re

def process_input(args):

    print(f'Processing file: {args.filename}')

    # Maximum or specified
    file = args.filename    
    rows = args.yrange #if args.yrange else "full"
    columns = args.xrange #if args.xrange else "full"

    # Check if the file exists
    if not os.path.isfile(args.filename):
        print(f"Error: The file '{args.filename}' does not exist.")
        return  # Exit the function if the file is not found

    # Validate index input formatting
    def validate_index(index):
        # Check if "full" is passed, case insensitive
        if index.lower() == "full":
            return index.lower()

        # Regex to match the "NUMBERtoNUMBER" format
        match = re.fullmatch(r"\d+to\d+", index)
        if match:
            return index  # Return as-is if valid

        # Raise an error if the input does not match any valid format
        raise ValueError(f"Invalid index format '{index}'. Expected 'NUMBERtoNUMBER' or 'full'.")

    validate_index(rows)
    validate_index(columns)

    # Prepare arguments to pass to the C function
    file = file.encode('utf-8')
    rows = rows.encode('utf-8')
    columns = columns.encode('utf-8')



    # Load the C library and define argument types
    matrix_lib = ctypes.CDLL('./shared_libraries/matrix_lib.so')
    matrix_lib.load_data.argtypes = [ctypes.c_char_p, ctypes.c_char_p, ctypes.c_char_p]
    matrix_lib.load_data.restype = ctypes.c_int

    # Call the C function that preparse the data for operation
    result = matrix_lib.load_data(file, rows, columns)
    print("Failure status:", result)

    operations = 0

    # Bitwise flag definitions
    MAX_FLAG = 1 << 0     # 000001 (1)
    MIN_FLAG = 1 << 1     # 000010 (2)
    MEAN_FLAG = 1 << 2    # 000100 (4)
    MEDIAN_FLAG = 1 << 3  # 001000 (8)
    MODE_FLAG = 1 << 4    # 010000 (16)
    STDDEV_FLAG = 1 << 5  # 100000 (32)

    # Set bitwise flags based on user input
    if args.max:
        operations |= MAX_FLAG
    if args.min:
        operations |= MIN_FLAG
    if args.mean:
        operations |= MEAN_FLAG
    if args.median:
        operations |= MEDIAN_FLAG
    if args.mode:
        operations |= MODE_FLAG
    if args.stddev:
        operations |= STDDEV_FLAG

    # Returns result of stat operation from shared library
    return "Successfully completed operation from shared library"

def main():
    # Create the main parser
    parser = argparse.ArgumentParser(description='Fat Data CLI for Statistical Operations')

    # Required argument: filename
    parser.add_argument('filename', help='Path to the data file (CSV, TSV, etc.)')

    # Optional arguments for x and y ranges in format x0tox2, y0toy2
    parser.add_argument('--xrange', help='Specify the x-range (column name or index)')
    parser.add_argument('--yrange', help='Specify the y-range (column name or index)')

    # Flags for max/min operation
    parser.add_argument('--max', action='store_true', help='Find the maximum of the selected dataset')
    parser.add_argument('--min', action='store_true', help='Find the minimum of the selected dataset')

    # Flags for statistical operations
    parser.add_argument('--mean', action='store_true', help='Calculate the mean of the dataset')
    parser.add_argument('--median', action='store_true', help='Calculate the median of the dataset')
    parser.add_argument('--mode', action='store_true', help='Calculate the mode of the dataset')
    parser.add_argument('--stddev', action='store_true', help='Calculate the standard deviation of the dataset')

    # Parse the arguments
    args = parser.parse_args()

    # Process the input and calculate result based on the requested operation
    result = process_input(args)    
    print(result)

if __name__ == "__main__":
    main()



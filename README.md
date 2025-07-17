# Fat Data Dataframe CLI
## A minimalistic CLI for statistical operations on structured CSV files
    - Supports:
        - Mean
        - Median
        - Mode
        - Max
        - Min
        - Standard Deviation
        - Input range selection

    - Reach features
        - Interactive range selection UI
        - Data output visualization 
        - ML optimized threaded job allocation
        - ML purposed arithmetic algorithms
        - API integration

### Implemented with C library for operation on arbitrarily sized data types
- Bash script runs the python and C library together
- The python calls a ctypes plugin for input handling
    - It calls a second plugin (the marshaller) that doles out arithmetic operations
        - Arithmetic operations are computed with the fat data api

#### Input sanitization

##### Initial input is sanitized in matrix_lib.C
<!-- ###### How are we sanitizing input? -->
- The python program takes range selection from flags in the form
    - --xrange
    - --yrange
    - It parses for input of the form
        - numbertonumber, nametoname, fulltofull, fulltonumber, numbertofull, and full
    - It then extracts by y1, y2, x1, x2, and passes them to the the plugin
- The plugin parses input by 
    - Storing provided numbers in a structure
        - A number of values are safe incremented to enforce graceful termination on overflow
    - Finding the position of named headers *and*
        - Storing them in an array of strings
    - Checking that the positions are within the bounds of the file content dimensions
        - And whether or not we found the header
    - Setting the starting values based on relative sizes, to ignore input ordering
    - Ensuring that the dimensions are greater than 2, but within integer sizing
    - Checking that header formatting is consistent
        - Allowing for both headers and columns, or exclusively columns or headers

# Marshaller Operations

# Additional issues
## Wordlist for arbitrary CSV file generation sourced from 
https://github.com/kkrypt0nn/wordlists/tree/main 

## To build the docker container
docker build -t fatdata-asan .

## To run inside docker container with Valgrind memory oversight
docker run --rm -it -v "$(pwd)":/app fatdata-asan

## Sample command
./dev_functionality/run_analysis.sh ./dataframes/example2.csv --xrange 1to5 --yrange 1to3 --max --mean

# ToDO
- Test edge cases on CSV dimensions
- Implement the Marshaller thread operation w/ fat data CLI

# Constraints, issues, and ideas for future development
- Single width of column data operations
- The bounds are limited by integer sizes
- Multiple reads of data for processing
- Memory mapped operations
- Vector API integration

# Target command
./dataframes/example2.csv full full full full



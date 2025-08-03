# 📊 Fat Data - A Dataframe CLI for CSV Analysis
*My shoddy introduction to fast compute systems programming.* A fast(ish), minimalistic command-line interface for performing threaded statistical operations on CSV data -- designed for arbitrarily large numbers and datasets. Built with a C backend for precision and performance, and wrapped with Python and Bash for usability.

## 🚀 Features
- Computes statistical metrics:
    - Mean, Median, Mode, Max, Min
- Flexible input range selection:
    - Supports formats like 1to5, header1toheader5, full, etc.
- Handles arbitrarily large numbers 
- Input sanitization and graceful error handling
- Pretty printed matrices with truncated output (to prevent wrapping)
- Easy Dockerized + Valgrind setup
    
## ⚙️ Architecture    
                ┌─────────────-┐
                │  User Input  │
                └─────-┬────-─-┘
                  Bash Script 
             ┌─────────┴──────────┐
             │                    │
    CLI (Python Entry)     Input generation
             │                    │
      ┌──────┴───────-┐     ┌─────┴──--─────┐
      │ Ctypes Plugin │     │ C hook driver │
      └──────┬───────-┘     └──────┬───-────┘
             │                     │
             └────------┬─--───-─--┘
                 ┌─────-┴────-┐
                 │ Matrix_lib │
                 └─────-┬──-─-┘
                 ┌─────-┴────-┐
                 │ Marshaller │
                 └─────-┬──-─-┘
                 ┌─────-┴────-┐
                 │  Fat_data  │
                 └─────---─-─-┘

### Execution modes
#### The Bash script compiles and links the necesary binaries. 
#### It checks for flags that request memory analysis via --memcheck

- Python CLI Mode (default)
    - Invoked by the bash script
    - Checks flag formatting and passes values to the Ctypes routine  
- Analysis via Valgrind
    - Bash invokes a script that generates random commands 
    - It then calls a C hook driver that invokes these commands
    - Bash runs each execution instance in a loop 
    - It then outputs valgrind results to a folder for that memcheck invocation
    - Note: 
        - Operations are passed explicitly as the bitwise integer (clunky, I know)    
        - Thread-count is passed explicitly    

## 🧼 Input sanitization
### Python layer
- Parses input range flags --xrange, --yrange
- Accepts: 
    - Numeric ranges (1to5)
    - Header based ranges (colAtoColE)
    - Full selections (default)
        ~ full in the first column adjusts to the first column, the second, the last
    - Converts flags to structured x/y boundaries
    - Generates a bitmasked integer denoting requested operations

### C Plugin Layer
- Validates numeric inputs and prevents overflow
- Checks bounds and dimensions
    - >= 2 by 2 matrices
    - Consistent row widths
    - Header formatting consistency:
        - Column headers, row headers, both, or neither
    - Ignores ordering
    - Validates against missing or out-of-bounds dimensions
    - Hands off requested subregion to the marshaller library

## 🧮 Marshaller Operations
- This layer handles threaded statistical operations and dispatches computation to the API
- Operations:
    - Max
    - Min
    - Mean
    - Median
    - Mode

## Usage

## To build the docker container
docker build -t fatdata-asan .

## To run inside docker container with Valgrind memory oversight
docker run --rm -it -v "$(pwd)":/app fatdata-asan

*(Optionally create a virtual environment and jump in there)*

### Sample command with default Python/Ctypes flow
./dev_functionality/run_analysis.sh ./dataframes/example2.csv --xrange 1to5 --yrange 1to3 --max --mean

### Sample command with memcheck flow
./dev_functionality/run_analysis.sh --memcheck --rerun --operations=7 --thread-count 3
./dev_functionality/run_analysis.sh --memcheck --operations=8 --thread-count 3

#### Note:
- Memcheck allows you to rerun with the previous batch of commands
    - This is done via --rerun

## 📈 Stress testing results
*I ran a script that generated some HUGE files, just to see when we exceed the plugin buffer or otherwise crash*
- Each parameter tested independently:
    - We can handle pretty darned big numbers
        - I got it to work on values with 26 digits
    - Testing column width by number of cells
        - It crashes between 10 and 100 million columns
            - This was run with two rows: 
                ~ 20-200 million values
    - Testing row width by character count:
        We crash at just about 10^9 characters in a row
    - Testing number of rows
        - We crash at about 10^8 rows
        - Also ran arbitrarily with 2 columns
            ~ 2 * 10^8 rows

Not bad for a user space application.

## 🔩 Constraints & Future Work
### I've got a LOT of ideas for a fork that implements my wishlist
- Gracefully handle:
    - 1 by 1 data dimensions
    - Unfilled or malformed cells
- Extended support to standard deviation and advanced stats
- Less clunky range selection parsing
- Floating point arithmetic operations
- Handle variables number bases (Octal, Hexadecimal, etc.)
- Offload printing to Python frontend for beautiful graphs
- Minimal scripting language to perform operations *between* CSVs:
    - Merge operations, arithmetic operations
- Implementation operations
    - CACHE FRIENDLINESS 
        - The largest epiphony I had 
            - Don't load a 2gb file into memory
        - I could write cache friendlier code by maximizing locality
            - Map code into memory
            - Inform the OS about sequential access
            - Utilize a ring buffer or sliding window
            - Minimize IO boundedness
    - Smarter threading
        - Producer consumer architecture
        - Locking mechanisms 
    - CUDA Backend?

### Wordlist used for file generation 
https://github.com/kkrypt0nn/wordlists/tree/main

*If you'd like to hear me ramble further about this project, and what I've learned, you can check out the devlog on my personal website [Elijah Dayney](https://personal-website-beta-ten-95.vercel.app/)*






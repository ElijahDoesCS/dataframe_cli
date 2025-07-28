#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>

#include "../arithmetic_lib/fat_data/fat_data.h"
#include "./marshaller/marshaller.h"
#include "../arithmetic_lib/hashmap/hashmap.h"

#define BUFFER_INCREMENT 64
#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))
#define SAFE_STRNDUP(src) strndup((src), INT_MAX - 1)

struct header_strings {
    char *starting_row;
    char *ending_row;
    char *starting_column;
    char *ending_column;
} typedef header_strings;

struct header_integers {
    int starting_row;
    int ending_row;
    int starting_column;
    int ending_column;    
} typedef header_integers;

typedef enum {
    STARTING_ROW,
    ENDING_ROW,
    STARTING_COLUMN,
    ENDING_COLUMN
} HeaderField;

//STRUCTURE FUNCTIONALITY

void free_header_strings(header_strings *header_strings) {
    if (header_strings->starting_row) free(header_strings->starting_row);
    if (header_strings->ending_row) free(header_strings->ending_row);
    if (header_strings->starting_column) free(header_strings->starting_column);
    if (header_strings->ending_column) free(header_strings->ending_column);
}

// Function to check if a specific field is set (not -1)
bool value_set(header_integers header_indeces, HeaderField field) {
    switch (field) {
        case STARTING_ROW:
            return header_indeces.starting_row != -1;
        case ENDING_ROW:
            return header_indeces.ending_row != -1;
        case STARTING_COLUMN:
            return header_indeces.starting_column != -1;
        case ENDING_COLUMN:
            return header_indeces.ending_column != -1;
        default:
            return false; // Invalid field
    }
}

// Values array functionality
void free_matrix(char **values, int values_size) {
    if (values) {
        for (int i = 0; i < values_size; i++) {
            if (values[i]) free(values[i]); 
        }
        free(values); 
    }
}

void pretty_print_values(char **values, int values_size, int data_width) {
    if (data_width <= 0 || values_size <= 0) return;
    int num_rows = values_size / data_width;

    // Calculate max width for each column
    int *col_widths = malloc(data_width * sizeof(int));
    for (int col = 0; col < data_width; col++) col_widths[col] = 0;
    for (int row = 0; row < num_rows; row++) {
        for (int col = 0; col < data_width; col++) {
            int idx = row * data_width + col;
            int len = strlen(values[idx]);
            if (len > col_widths[col]) col_widths[col] = len;
        }
    }

    // Print top border
    printf("┌");
    for (int col = 0; col < data_width; col++) {
        for (int w = 0; w < col_widths[col] + 2; w++) printf("─");
        if (col < data_width - 1) printf("┬");
    }
    printf("┐\n");

    // Print rows
    for (int row = 0; row < num_rows; row++) {
        printf("│");
        for (int col = 0; col < data_width; col++) {
            int idx = row * data_width + col;
            printf(" %-*s │", col_widths[col], values[idx]);
        }
        printf("\n");

        // Print separator
        if (row < num_rows - 1) {
            printf("├");
            for (int col = 0; col < data_width; col++) {
                for (int w = 0; w < col_widths[col] + 2; w++) printf("─");
                if (col < data_width - 1) printf("┼");
            }
            printf("┤\n");
        }
    }

    // Print bottom border
    printf("└");
    for (int col = 0; col < data_width; col++) {
        for (int w = 0; w < col_widths[col] + 2; w++) printf("─");
        if (col < data_width - 1) printf("┴");
    }
    printf("┘\n");

    free(col_widths);
}

// OVERFLOW OPERATORS
bool safe_size_t_increment(size_t *value) {
    if (*value == SIZE_MAX) {
        fprintf(stderr, "Error: Maximum number of values (%d) exceeded.\n", INT_MAX);
        return false;
    }
    (*value)++;
    return true;
}

bool safe_increment(int *value) {
    if (*value >= INT_MAX - 1) {
        fprintf(stderr, "Error: Maximum number of values (%d) exceeded.\n", INT_MAX);
        return false;
    }
    (*value)++;

    return true;
}

// Tokenize and store line
bool process_line(const char *line, 
    header_strings requested_headers, header_integers *header_indeces, 
    char ***values, size_t *values_size, 
    int num_lines, int *data_width, bool first_line) {
    
    char *line_copy = SAFE_STRNDUP(line); // For safe tokenization
    if (!line_copy) {
        perror("Memory allocation failed");
        return false;
    }

    const char *delimiters = ",;|";
    char *token = strtok(line_copy, delimiters);

    int current_width = 0;
    bool headers_matched = true;

    while (token) {
        if (!safe_increment(&current_width)) return false;

        // Look for the requested headers in the tokens
        if (first_line) {
            if (current_width == 1) {
                
                // Consider whether we have already found the header, i.e. do not allow for repeat headers
                if (requested_headers.starting_column &&
                    strcmp(token, requested_headers.starting_column) == 0) {                    
                    if (value_set(*header_indeces, STARTING_COLUMN)) {
                        fprintf(stderr, "Error: Repeat header \"%s\"\n", token);
                        free(line_copy);
                        return false;
                    }

                    header_indeces->starting_column = current_width - 1;
                }
                if (requested_headers.ending_column &&
                    strcmp(token, requested_headers.ending_column) == 0) {
                    if (value_set(*header_indeces, ENDING_COLUMN)) {
                        fprintf(stderr, "Error: Repeat header \"%s\"\n", token);
                        free(line_copy);
                        return false;
                    }

                    header_indeces->ending_column = current_width - 1;
                }
                if (requested_headers.starting_row &&
                    strcmp(token, requested_headers.starting_row) == 0) {
                    if (value_set(*header_indeces, STARTING_ROW)) {
                        fprintf(stderr, "Error: Repeat header \"%s\"\n", token);
                        free(line_copy);
                        return false;
                    }

                    header_indeces->starting_row = num_lines + 1;
                }
                if (requested_headers.ending_row && 
                    strcmp(token, requested_headers.ending_row) == 0) {
                    if (value_set(*header_indeces, ENDING_ROW)) {
                        fprintf(stderr, "Error: Repeat header \"%s\"\n", token);
                        free(line_copy);
                        return false;
                    }

                    header_indeces->ending_row = num_lines + 1;
                }
            } else if (current_width >= 2) {
                if (requested_headers.starting_column &&
                    strcmp(token, requested_headers.starting_column) == 0) {
                    if (value_set(*header_indeces, STARTING_COLUMN)) {
                        fprintf(stderr, "Error: Repeat header \"%s\"\n", token);
                        free(line_copy);
                        return false;
                    }

                    header_indeces->starting_column = current_width - 1;
                }
                if (requested_headers.ending_column &&
                    strcmp(token, requested_headers.ending_column) == 0) {
                    if (value_set(*header_indeces, ENDING_COLUMN)) {
                        fprintf(stderr, "Error: Repeat header \"%s\"\n", token);
                        free(line_copy);
                        return false;
                    }

                    header_indeces->ending_column = current_width - 1;
                }
            }
        } else if (current_width == 1) {
            // Print the row header if it matches
            if (requested_headers.starting_row &&
                strcmp(token, requested_headers.starting_row) == 0) {
                if (value_set(*header_indeces, STARTING_ROW)) {
                    fprintf(stderr, "Error: Repeat header \"%s\"\n", token);
                    free(line_copy);
                    return false;
                }

                header_indeces->starting_row = num_lines + 1;
            }
            if (requested_headers.ending_row && 
                strcmp(token, requested_headers.ending_row) == 0) {
                if (value_set(*header_indeces, ENDING_ROW)) {
                    fprintf(stderr, "Error: Repeat header \"%s\"\n", token);
                    free(line_copy);
                    return false;
                }

                header_indeces->ending_row = num_lines + 1;
            }
        }

        // We have exceeded the number of strings allocated for the buffer
        if (*values_size % BUFFER_INCREMENT == 0) {
            char **new_values = realloc(*values, (*values_size + BUFFER_INCREMENT) * sizeof(char *));
            if (!new_values) {
                perror("Memory reallocation failed");
                free(line_copy);
                return false;
            }
            *values = new_values;
        }

        // Add the token to the values array
        (*values)[*values_size] = SAFE_STRNDUP(token);
        if (!(*values)[*values_size]) {
            perror("Memory allocation failed");
            free(line_copy);
            return false;
        }

        // (*values_size)++;
        if (!safe_size_t_increment(values_size)) return false;

        token = strtok(NULL, delimiters);
    }

    // Ensure data is aligned properly
    if (first_line) {
        *data_width = current_width;
    } else if (current_width != *data_width) {
        fprintf(stderr, "Error: Line width (%d) does not match the expected width (%d)\n", current_width, *data_width);
        fprintf(stderr, "Line prefix: %s, Row number: %d\n", line_copy, (num_lines + 1));
        free(line_copy);
        return false;
    }

    free(line_copy); // Free the duplicated line
    return true;
}

// Store file contents and verify data alignment
char **tokenize_file_contents(const char *file_name, 
    header_strings requested_headers, header_integers *header_indeces,
    int *store_data_width, int *store_num_lines, int *store_num_values) {    

    FILE *spreadsheet_fp;
    spreadsheet_fp = fopen(file_name, "r");

    if (spreadsheet_fp == NULL) { 
        fprintf(stderr, "Error opening and parsing file contents (corrupted file pointer).\n");
        return NULL;
    }
    
    char **values = NULL;
    size_t values_size = 0; // Hold number of values in values arr

    int buffer_size = BUFFER_INCREMENT;
    char *line = malloc(buffer_size); // Start with a line buffer of explicit size
    if (!line) {
        perror("Memory allocation failed");
        fclose(spreadsheet_fp);
        
        return NULL;
    }

    int line_size = 0; // Number of characters in the current line
    signed char ch; // Keep upper bits

    bool first_line = true; // Store data width on first line
    int data_width = 0, num_lines = 0; 
    while ((ch = fgetc(spreadsheet_fp)) != EOF) {
        // Acquired line
        if (ch == '\n') {
            line[line_size] = '\0'; // Null-terminate the string

            // Add line to values array
            if (!process_line(line, requested_headers, header_indeces, 
                              &values, &values_size, 
                              num_lines,  &data_width, first_line)) {

                fprintf(stderr, "File format error detected.\n");
                free(line);
                fclose(spreadsheet_fp);

                free_matrix(values, values_size);
                return NULL;
            }

            // Reset for the next line
            line_size = 0;

            first_line = false;

            if (!safe_increment(&num_lines)) return NULL;
            continue;
        }

        // If we have exceeded the buffer size
        if (line_size >= buffer_size - 1) { // Room for null terminator
            if (INT_MAX - buffer_size > BUFFER_INCREMENT) {
                buffer_size += BUFFER_INCREMENT;
            }
            else {
                fprintf(stderr, "Error: Maximum number of values (%d) exceeded.\n", INT_MAX);
                return NULL;
            }

            char *new_line = realloc(line, buffer_size);
            if (!new_line) {
                perror("Memory reallocation failed");
                free(line);
                fclose(spreadsheet_fp);
                free_matrix(values, values_size);
                return NULL;
            }
    
            line = new_line;
        }

        // Add the character to the buffer        
        line[line_size] = ch;
        if (!safe_increment(&line_size)) return NULL;
    }

    // Operate on final line
    if (line_size > 0) {
        line[line_size] = '\0'; // Null-terminate the string
        if (!process_line(line, requested_headers, header_indeces, 
                              &values, &values_size, 
                              num_lines,  &data_width, first_line)) {
            fprintf(stderr, "File format error detected.\n");
            free(line);
            fclose(spreadsheet_fp);
            free_matrix(values, values_size);
            return NULL;
        }
        num_lines++;
    }

    *store_data_width = data_width;
    *store_num_lines = num_lines;
    *store_num_values = values_size;

    free(line);
    fclose(spreadsheet_fp);

    return values;
}

__attribute__((visibility("default"))) int load_data(const char *file_name,
    const char *starting_row, const char *ending_row, 
    const char *starting_column, const char *ending_column,
    int operations,
    int thread_count) {

    // Variables to store integer or string interpretations
    int starting_row_int = -1, ending_row_int = -1;
    int starting_column_int = -1, ending_column_int = -1;

    char *starting_row_string = NULL, *ending_row_string = NULL;
    char *starting_column_string = NULL, *ending_column_string = NULL;

    #define CONVERT_IF_NUMERIC(input, output_int, output_string)              \
        do {                                                                  \
            char *endptr = NULL;                                              \
            errno = 0;                                                        \
            long val = strtol((input), &endptr, 10);                          \
            if (errno == 0 && endptr != input && *endptr == '\0' &&           \
                val >= INT_MIN && val <= INT_MAX) {                           \
                (output_int) = (int)(val - 1);                                \
            } else {                                                          \
                (output_string) = SAFE_STRNDUP(input);                        \
            }                                                                 \
        } while (0)

    // Store header indeces in integer structure
    CONVERT_IF_NUMERIC(starting_row, starting_row_int, starting_row_string);
    CONVERT_IF_NUMERIC(ending_row, ending_row_int, ending_row_string);
    CONVERT_IF_NUMERIC(starting_column, starting_column_int, starting_column_string);
    CONVERT_IF_NUMERIC(ending_column, ending_column_int, ending_column_string);

    // Populate strings for header search
    header_strings header_strings = {
        .starting_row = starting_row_string,
        .ending_row= ending_row_string,
        .starting_column = starting_column_string,
        .ending_column = ending_column_string
    };

    // Populate the header_integers structure for subregion buildout
    header_integers header_integers = {
        .starting_row = (header_strings.starting_row) ? -1 : starting_row_int,
        .ending_row = (header_strings.ending_row) ? -1 : ending_row_int,
        .starting_column = (header_strings.starting_column) ? -1 : starting_column_int,
        .ending_column = (header_strings.ending_column) ? -1 : ending_column_int
    };

    /*
    Hold data, get dimensions, number of values, and validate width consistency across rows
    Populate the header_integers structure with retrieved headers
    */ 

    int data_width = 0, num_lines = 0, values_size = 0; // Num columns, num rows, num tokens
    char **values = tokenize_file_contents(file_name, header_strings, &header_integers, 
                                          &data_width, &num_lines, &values_size);
    if (!values) {
        fprintf(stderr, "Error opening and parsing file contents. (Void file contents)\n");
        
        // Free allocated memory for header strings
        free_header_strings(&header_strings);
        
        return 1;
    }

    // Verify spreadsheet dimensions
    if (data_width < 2 || num_lines < 2) {
        free_matrix(values, values_size);
        free_header_strings(&header_strings);

        fprintf(stderr, "Error: Expects >=2 by >=2 dimensions in CSV file.\n");
        fprintf(stderr, "Dimensions (height by width): %d by %d\n", num_lines, data_width);
        fprintf(stderr, "File format error detected.\n");
        
        return 1;
    }

    /*
    Reject inputs that have mixed headers, i.e. numbers in column headers, 
    numbers in row headers
    */ 

    bool column_headers = true;
    for (int i = 0; i < data_width; i++) {
        if (is_valid_double(values[i])) {
            column_headers = false;
            break;
        }
    }

    // We don't have column headers, i.e. not all strings
    if (!column_headers) {
        // The first can be a string or a number, the rest have to be numbers
        for (int i = 1; i < data_width; i++) {
            // Not all numbers 
            if (!is_valid_double(values[i])) {
                fprintf(stderr, "Error: Data formatting expects headers to be numerical or lexicographical.\n");
                fprintf(stderr, "Mixed formatting: column header\n");
                fprintf(stderr, "File format error detected.\n");
                
                free_matrix(values, values_size);
                free_header_strings(&header_strings);

                return 1;
            }
        }
    }

    // Verify row header formatting
    bool row_headers = true;
    for (int i = 0; i <= values_size - data_width; i += data_width) { 
        if (is_valid_double(values[i])) {
            row_headers = false;
            break;
        }
    }

    // We don't have row headers, i.e. not all strings
    if (!row_headers) {
        for (int i = data_width; i <= values_size - data_width; i += data_width) {
            // Not all numbers
            if (!is_valid_double(values[i])) {
                fprintf(stderr, "Error: Data formatting expects headers to be numerical or lexicographical.\n");
                fprintf(stderr, "Mixed formatting: row headers\n");
                fprintf(stderr, "File format error detected.\n");
                
                free_matrix(values, values_size);
                free_header_strings(&header_strings);

                return 1;
            }
        }

        if (!column_headers && !is_valid_double(values[0])) {
            fprintf(stderr, "Error: Data formatting expects headers to be numerical or lexicographical.\n");
            fprintf(stderr, "Mixed formatting: column headers\n");
            fprintf(stderr, "File format error detected.\n");
            
            free_matrix(values, values_size);
            free_header_strings(&header_strings);
            
            return 1;
        } 
    }

    // Adjust initial bounds if column headers are present
    if (column_headers) {
        // We skip the first row for data if there's a column header
        if (header_integers.starting_row != -1 & header_strings.starting_row == NULL) header_integers.starting_row++;
        if (header_integers.ending_row != -1 & header_strings.ending_row == NULL) header_integers.ending_row++;

        // If the user specified "full", adjust accordingly
        if (header_strings.starting_row && strcmp(header_strings.starting_row, "full") == 0) {
            header_integers.starting_row = 1;  // start after the header row
        }

        if (header_strings.ending_row && strcmp(header_strings.ending_row, "full") == 0) {
            // Make sure we don't exceed file bounds
            header_integers.ending_row = num_lines - 1;
        }
    }

    // Adjust bounds if row headers are present
    if (row_headers) {
        // Skip the first column if there's a row header
        if (header_integers.starting_column != -1 & header_strings.starting_column == NULL) header_integers.starting_column++;
        if (header_integers.ending_column != -1 & header_strings.ending_column == NULL) header_integers.ending_column++;

        // If the user specified "full" for columns
        if (header_strings.starting_column && strcmp(header_strings.starting_column, "full") == 0) {
            header_integers.starting_column = 1;  // skip header column
        }

        if (header_strings.ending_column && strcmp(header_strings.ending_column, "full") == 0) {
            header_integers.ending_column = data_width - 1;
        }
    }

    // General fallback (if headers weren't present, still handle "full")
    if (!column_headers) {
        if (header_strings.starting_row && strcmp(header_strings.starting_row, "full") == 0) {
            header_integers.starting_row = 0;
        }
        if (header_strings.ending_row && strcmp(header_strings.ending_row, "full") == 0) {
            header_integers.ending_row = num_lines - 1;
        }
    }
    if (!row_headers) {
        if (header_strings.starting_column && strcmp(header_strings.starting_column, "full") == 0) {
            header_integers.starting_column = 0;
        }
        if (header_strings.ending_column && strcmp(header_strings.ending_column, "full") == 0) {
            header_integers.ending_column = data_width - 1;
        }
    }

    // If we can't find one of the requested headers or they are out of bounds
    if (header_integers.starting_row <= -1 || 
        header_integers.ending_row <= -1 ||
        header_integers.starting_column <= -1 || 
        header_integers.ending_column <= -1 ||
        header_integers.starting_row >= num_lines ||
        header_integers.ending_row >= num_lines || 
        header_integers.starting_column >= data_width || 
        header_integers.ending_column >= data_width
    ) {
        fprintf(stderr, "Error: Requested dimensions not found or exceeded dimensions (indexed from 0).\n");

        if (header_integers.starting_row == -1 || header_integers.starting_row >= num_lines) {
            header_strings.starting_row ? 
                fprintf(stderr, "   Requested starting row: %s\n", header_strings.starting_row) : 
                fprintf(stderr, "   Requested starting row: %d\n", header_integers.starting_row);
                fprintf(stderr, "   File number of row(s): %d\n", num_lines);
        }
        if (header_integers.ending_row == -1 || header_integers.ending_row >= num_lines) {
            header_strings.ending_row ? 
                fprintf(stderr, "   Requested ending row: %s\n", header_strings.ending_row) : 
                fprintf(stderr, "   Requested ending row: %d\n", header_integers.ending_row);
                fprintf(stderr, "   File number of row(s): %d\n", num_lines);
        }
        if (header_integers.starting_column == -1 || header_integers.starting_column >= data_width) {
            header_strings.starting_column ? 
                fprintf(stderr, "   Requested starting column: %s\n", header_strings.starting_column) : 
                fprintf(stderr, "   Requested starting column: %d\n", header_integers.starting_column);
                fprintf(stderr, "   File number of columns: %d\n", data_width);
        }
        if (header_integers.ending_column == -1 || header_integers.ending_column >= data_width) {
            header_strings.ending_column ? 
                fprintf(stderr, "   Requested ending column: %s\n", header_strings.ending_column) : 
                fprintf(stderr, "   Requested ending column: %d\n", header_integers.ending_column);
                fprintf(stderr, "   File number of columns: %d\n", data_width);
        }

        // Free allocated memory
        free_matrix(values, values_size);
        free_header_strings(&header_strings);

        // Exit with error
        return 1;
    }

    // Switch values if the first or last need to be switched, i.e. end > start
    int starting_row_copy = header_integers.starting_row;
    int ending_row_copy = header_integers.ending_row;
    int starting_column_copy = header_integers.starting_column;
    int ending_column_copy = header_integers.ending_column;
    
    // Set these in order
    header_integers.starting_row = min(starting_row_copy, ending_row_copy);
    header_integers.ending_row = max(starting_row_copy, ending_row_copy);
    header_integers.starting_column = min(starting_column_copy, ending_column_copy);
    header_integers.ending_column = max(starting_column_copy, ending_column_copy);

    // Free the strings allocated to store requested headers
    free_header_strings(&header_strings);

    // Grab values from the array
    int sub_width = (header_integers.ending_column - header_integers.starting_column) + 1;
    int sub_height = (header_integers.ending_row - header_integers.starting_row) + 1;
    char **subregion = malloc(sizeof(char *) * sub_width * sub_height);
    if (!subregion) {
        perror("malloc failed for subregion");
        exit(EXIT_FAILURE);
    }

    // Build out the subregion from the values array
    int i = 0;
    for (int row = 0; row < sub_height; row++) {
        for (int col = 0; col < sub_width; col++) {
            int original_index = (header_integers.starting_row + row) * data_width + (header_integers.starting_column + col);

            size_t len = strlen(values[original_index]) + 1;
            subregion[i] = malloc(len);
            if (!subregion[i]) {
                fprintf(stderr, "Memory allocation failed at subregion handoff\n");
                free_matrix(values, values_size);
                return 1;
            }
            memcpy(subregion[i], values[original_index], len);
            i++;
        }
    }

    int subregion_size = sub_height * sub_width;

    // Pretty print the input data
    pretty_print_values(values, values_size, data_width);

    free_matrix(values, values_size);

    // // Send out the operations on the subregion to be performed across threads
    // int marshaller = marshall_operations(subregion, sub_height, sub_width, subregion_size, operations, thread_count);
    // if (marshaller) {
    //     fprintf(stderr, "Error: marshall_operations failed to compute operation (returned %d)\n", marshaller);
    //     free_matrix(subregion, sub_height * sub_width);     
    //     return 1;
    // }

    free_matrix(subregion, sub_height * sub_width);
 
    return 0;
}




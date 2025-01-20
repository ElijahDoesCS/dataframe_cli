#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

#define BUFFER_INCREMENT 64

struct header_strings {
    const char *starting_row;
    const char *ending_row;
    const char *starting_column;
    const char *ending_column;
} typedef header_strings;

struct header_integers {
    int starting_row;
    int ending_row;
    int starting_column;
    int ending_column;    
} typedef header_integers;

// Returns true if string is a valid double, false if not
bool is_double(const char *token) {
    char *endptr;
    strtod(token, &endptr);

    return *endptr == '\0';
}

// Cleanup values array
void free_values(char **values, int values_size) {
    if (values) {
        for (int i = 0; i < values_size; i++) {
            free(values[i]); 
        }
        free(values); 
    }
}

// Tokenize and store line
bool process_line(const char *line, header_strings requested_headers, char ***values, int *values_size, int *data_width, bool first_line) {
    char *line_copy = strdup(line); // For safe tokenization
    if (!line_copy) {
        perror("Memory allocation failed");
        return false;
    }

    const char *delimiters = ",;|";
    char *token = strtok(line_copy, delimiters);

    int current_width = 0;
    bool headers_matched = true;

    while (token) {
        current_width++;

        if (first_line) {
            if (current_width == 1) {
                
                // For the very first value, check all the strings
                if (requested_headers.starting_column &&
                    strcmp(token, requested_headers.starting_column) == 0) {
                    printf("Starting column string found at x = %d\n", current_width);
                }
                if (requested_headers.ending_column &&
                    strcmp(token, requested_headers.ending_column) == 0) {
                    printf("Ending column string found at x = %d\n", current_width);
                }
                if (requested_headers.starting_row &&
                    strcmp(token, requested_headers.starting_row) == 0) {
                    printf("Starting row string found at x = %d", current_width);
                }
                if (requested_headers.ending_row && 
                    strcmp(token, requested_headers.ending_row) == 0) {
                    printf("Ending row string found at x = %d\n", current_width);
                }
            } else if (current_width >= 2) {
                // Check if we find the starting or ending column header 
                if (requested_headers.starting_column &&
                    strcmp(token, requested_headers.starting_column) == 0) {
                    printf("Starting column string found at x = %d\n", current_width);
                }
                if (requested_headers.ending_column &&
                    strcmp(token, requested_headers.ending_column) == 0) {
                    printf("Ending column string found at x = %d\n", current_width);
                }
            }
        } else if (current_width == 1) {
            // Print the row header if it matches
            if (requested_headers.starting_row &&
                strcmp(token, requested_headers.starting_row) == 0) {

                printf("Starting row string found at x = %d", current_width);
            }
            if (requested_headers.ending_row && 
                strcmp(token, requested_headers.ending_row) == 0) {

                printf("Ending row string found at x = %d\n", current_width);
            }
        }
        

        // Resize values array if needed
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
        (*values)[*values_size] = strdup(token);
        if (!(*values)[*values_size]) {
            perror("Memory allocation failed");
            free(line_copy);
            return false;
        }
        (*values_size)++;

        token = strtok(NULL, delimiters);
    }

    // Ensure data is aligned properly
    if (first_line) {
        *data_width = current_width;
    } else if (current_width != *data_width) {
        fprintf(stderr, "Error: Line width (%d) does not match the expected width (%d)\n", current_width, *data_width);
        fprintf(stderr, "Line: %s\n", line_copy);
        free(line_copy);
        return false;
    }

    free(line_copy); // Free the duplicated line
    return true;
}

// Store file contents and verify data alignment
char **tokenize_file_contents(const char *file_name, 
    header_strings requested_headers, 
    int *store_data_width, int *store_num_lines, int *store_num_values) {    

    FILE *spreadsheet_fp;
    spreadsheet_fp = fopen(file_name, "r");

    if (spreadsheet_fp == NULL) { 
        fprintf(stderr, "Error opening and parsing file contents.\n");
        return NULL;
    }
    
    char **values = NULL;
    int values_size = 0; // Hold number of values in values arr

    size_t buffer_size = BUFFER_INCREMENT;
    char *line = malloc(buffer_size); // Start with a line buffer of size 1024
    if (!line) {
        perror("Memory allocation failed");
        fclose(spreadsheet_fp);
        free_values(values, values_size);
        return NULL;
    }

    size_t line_size = 0; // Number of characters in the current line
    char ch;

    bool first_line = true; // Store data width on first line
    int data_width = 0, num_lines = 0;
    while ((ch = fgetc(spreadsheet_fp)) != EOF) {
        // Acquired line
        if (ch == '\n') {
            line[line_size] = '\0'; // Null-terminate the string

            // Add line to values array
            if (!process_line(line, requested_headers, &values, &values_size, &data_width, first_line)) {
                fprintf(stderr, "File format error detected.\n");
                free(line);
                fclose(spreadsheet_fp);
                free_values(values, values_size);
                return NULL;
            }

            // Reset for the next line
            line_size = 0;

            first_line = false;
            num_lines++;
            continue;
        }

        // Ensure we have enough space
        if (line_size >= buffer_size - 1) { // Room for null terminator
            buffer_size += BUFFER_INCREMENT;
            char *new_line = realloc(line, buffer_size);
            if (!new_line) {
                perror("Memory reallocation failed");
                free(line);
                fclose(spreadsheet_fp);
                free_values(values, values_size);
                return NULL;
            }
    
            line = new_line;
        }

        // Add the character to the buffer
        line[line_size++] = ch;
    }

    // Operate on final line
    if (line_size > 0) {
        line[line_size] = '\0'; // Null-terminate the string
        if (!process_line(line, requested_headers, &values, &values_size, &data_width, first_line)) {
            fprintf(stderr, "File format error detected.\n");
            free(line);
            fclose(spreadsheet_fp);
            free_values(values, values_size);
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
    const char *starting_column, const char *ending_column) {

    // Variables to store integer or string interpretations
    int starting_row_int = 0, ending_row_int = 0;
    int starting_column_int = 0, ending_column_int = 0;

    char *starting_row_string = NULL, *ending_row_string = NULL;
    char *starting_column_string = NULL, *ending_column_string = NULL;

    #define CONVERT_IF_NUMERIC(input, output_int, output_string) \
        if (is_double(input)) { \
            output_int = atoi(input); \
        } else { \
            output_string = strdup(input); \
        }

    // Validate requested bounds & obtain header indeces
    CONVERT_IF_NUMERIC(starting_row, starting_row_int, starting_row_string);
    CONVERT_IF_NUMERIC(ending_row, ending_row_int, ending_row_string);
    CONVERT_IF_NUMERIC(starting_column, starting_column_int, starting_column_string);
    CONVERT_IF_NUMERIC(ending_column, ending_column_int, ending_column_string);

    // Populate the header_strings structure
    header_strings header_strings = {
        .starting_row = starting_row_string,
        .ending_row= ending_row_string,
        .starting_column = starting_column_string,
        .ending_column = ending_column_string
    };

    // Print the contents of the header_strings structure
    printf("Starting Row: %s\n", header_strings.starting_row);
    printf("Ending Row: %s\n", header_strings.ending_row);
    printf("Starting Column: %s\n", header_strings.starting_column);
    printf("Ending Column: %s\n", header_strings.ending_column);

    // Populate the header_integers structure
    header_integers header_integers = {
        .starting_row = (header_strings.starting_row) ? -1 : starting_row_int,
        .ending_row = (header_strings.ending_row) ? -1 : ending_row_int,
        .starting_column = (header_strings.starting_column) ? -1 : starting_column_int,
        .ending_column = (header_strings.ending_column) ? -1 : ending_column_int
    };

    // Hold spreadsheet data
    int data_width = 0, num_lines = 0, values_size = 0; // Num columns, num rows, num tokens
    char **values = tokenize_file_contents(file_name, header_strings, &data_width, &num_lines, &values_size);
    if (!values) {
        fprintf(stderr, "Error opening and parsing file contents.\n");
        return 1;
    }

    // Free allocated memory for strings
    free(starting_row_string);
    free(ending_row_string);
    free(starting_column_string);
    free(ending_column_string);

    // Verify spreadsheet dimensions
    if (data_width < 2 || num_lines < 2) {
        fprintf(stderr, "Error: Expects 2 by 2 dimensions in CSV file.\n");
        fprintf(stderr, "Dimensions (height by width): %d by %d\n", num_lines, data_width);
        fprintf(stderr, "File format error detected.\n");
        
        free_values(values, values_size);
        return 1;
    }

    // Verify column and row header formatting consistency
    bool column_headers = true;
    for (int i = 0; i < data_width; i++) {
        if (is_double(values[i])) {
            column_headers = false;
            break;
        }
    }

    // We don't have column headers, i.e. not all strings
    if (!column_headers) {
        // The first can be a string or a number, the rest have to be numbers
        for (int i = 1; i < data_width; i++) {
            // Not all numbers 
            if (!is_double(values[i])) {
                fprintf(stderr, "Error: Data formatting expects headers to be numerical or lexicographical.\n");
                fprintf(stderr, "Mixed formatting: column header\n");
                fprintf(stderr, "File format error detected.\n");
                free_values(values, values_size);
                return 1;
            }
        }
    }

    // Verify row header formatting
    bool row_headers = true;
    for (int i = 0; i <= values_size - data_width; i += data_width) { 
        if (is_double(values[i])) {
            row_headers = false;
            break;
        }
    }

    // We don't have row headers, i.e. not all strings
    if (!row_headers) {
        for (int i = data_width; i <= values_size - data_width; i += data_width) {
            // Not all numbers
            if (!is_double(values[i])) {
                fprintf(stderr, "Error: Data formatting expects headers to be numerical or lexicographical.\n");
                fprintf(stderr, "Mixed formatting: row headers\n");
                fprintf(stderr, "File format error detected.\n");
                free_values(values, values_size);
                return 1;
            }
        }

        if (!column_headers && !is_double(values[0])) {
            fprintf(stderr, "Error: Data formatting expects headers to be numerical or lexicographical.\n");
            fprintf(stderr, "Mixed formatting: column headers\n");
            fprintf(stderr, "File format error detected.\n");
            free_values(values, values_size);
            return 1;
        } 
    }

    // Print values array
    printf("\nProcessed Values:\n");
    for (int i = 0; i < values_size; i++) {
        printf("%s ", values[i]);
        if ((i + 1)  % data_width == 0 && i != 0) printf("\n");
        free(values[i]);
    }

    free(values);

    return 0;
}



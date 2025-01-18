#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <stdbool.h>
#include <ctype.h>

// Returns true if string is a valid double, false if not
bool is_double(const char *token) {
    char *endptr;
    strtod(token, &endptr);

    // If endptr points to the null terminator, the entire string was valid
    return *endptr == '\0';
}

// Tokenize line of file and store it in values matrix
bool process_line(const char *line, char ***values, int *values_size, int *data_width, bool first_line) {
    char *line_copy = strdup(line); // Duplicate line for safe tokenization
    if (!line_copy) {
        perror("Memory allocation failed");
        return false;
    }

    const char *delimiters = ",;|";
    char *token = strtok(line_copy, delimiters);

    int current_width = 0;
    while (token) {
        current_width++;

        // Resize values array if needed
        if (*values_size % 1024 == 0) {
            char **new_values = realloc(*values, (*values_size + 1024) * sizeof(char *));
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
        fprintf(stderr, "Error: Line width (%d) does not match the expected width (%d).\n", current_width, *data_width);
        fprintf(stderr, "Line: %s\n", line_copy);
        return false;
    }

    free(line_copy); // Free the duplicated line

    return true;
}

int tokenize_file_contents(FILE *spreadsheet_fp, char ***values) {
    return false;
}

__attribute__((visibility("default"))) int load_data(const char *file_name, const char *starting_row, const char *ending_row, const char *starting_column, const char *ending_column) {

    // Start by opening the file
    FILE *spreadsheet_fp;
    spreadsheet_fp = fopen(file_name, "r");

    if (spreadsheet_fp == NULL) { 
        perror("Error opening and parsing file contents\n");
        return 1;
    }

    // Hold spreadsheet data
    char **values = NULL;
    int values_size = 0;

    size_t reset_buffer = 1024;
    size_t buffer_size = reset_buffer;
    char *line = malloc(buffer_size); // Start with a line buffer of size 1024
    if (!line) {
        perror("Memory allocation failed");
        fclose(spreadsheet_fp);
        return 1;
    }

    size_t line_size = 0; // Number of characters in the current line
    char ch;

    bool first_line = true; // Store data width on first line
    int data_width = 0, num_lines = 0;
    while ((ch = fgetc(spreadsheet_fp)) != EOF) {
        // We have the whole line
        if (ch == '\n') {
            line[line_size] = '\0'; // Null-terminate the string

            if (!process_line(line, &values, &values_size, &data_width, first_line)) {
                fprintf(stderr, "File format error detected.\n");
                free(line);
                fclose(spreadsheet_fp);
                free(values);
                return 1;
            }

            // Reset for the next line
            buffer_size = reset_buffer;
            line_size = 0;

            first_line = false;
            num_lines++;
            continue;
        }

        // Ensure we have enough space
        if (line_size >= buffer_size - 1) { // Leave room for null terminator
            buffer_size++;
            char *new_line = realloc(line, buffer_size);
            if (!new_line) {
                perror("Memory reallocation failed");
                free(values);
                free(line);
                fclose(spreadsheet_fp);
                return 1;
            }
    
            line = new_line;
        }

        // Add the character to the buffer
        line[line_size++] = ch;
    }

    // Operate on final line
    if (line_size > 0) {
        line[line_size] = '\0'; // Null-terminate the string
        if (!process_line(line, &values, &values_size, &data_width, first_line)) {
            fprintf(stderr, "File format error detected.\n");
            free(values);
            free(line);
            fclose(spreadsheet_fp);
            return 1;
        }
        num_lines++;
    }

    if (data_width < 2 || num_lines < 2) {
        fprintf(stderr, "Error: Data preperation expects 2 by 2 dimensions in CSV file.\n");
        fprintf(stderr, "Dimensions (height by width): %d by %d\n", num_lines, data_width);
        fprintf(stderr, "File format error detected.\n");
        free(values);
        free(line);
        fclose(spreadsheet_fp);
        return 1;
    }

    // Data is aligned, at leat 2x2, now check for column and row headers
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
                free(values);
                free(line);
                fclose(spreadsheet_fp);
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

    // If we have column headers and we have row headers, but the first value is numerical reject

    // if we don't have row headers they are all numerical except maybe the first

    // We don't have row headers, i.e. not all strings
    if (!row_headers) {
        for (int i = data_width; i <= values_size - data_width; i += data_width) {
            // Not all numbers
            if (!is_double(values[i])) {
                fprintf(stderr, "Error: Data formatting expects headers to be numerical or lexicographical.\n");
                fprintf(stderr, "Mixed formatting: row headers\n");
                fprintf(stderr, "File format error detected.\n");
                free(values);
                free(line);
                fclose(spreadsheet_fp);
                return 1;
            }
        }

        if (!column_headers && !is_double(values[0])) {
            fprintf(stderr, "Error: Data formatting expects headers to be numerical or lexicographical.\n");
            fprintf(stderr, "Mixed formatting: column headers\n");
            fprintf(stderr, "File format error detected.\n");
            free(values);
            free(line);
            fclose(spreadsheet_fp);
            return 1;
        } 
    }

    int starting_row_int, ending_row_int, starting_column_int, ending_column_int;
    
    // Validate and interpret row bounds
    if (strcmp(starting_row, "full") != 0) {
        starting_row_int = atoi(starting_row);
        ending_row_int = atoi(ending_row);

        // printf("Rows from %d to %d\n", starting_row_int, ending_row_int);
    }
    else {
        // printf("Full row\n");
        starting_row_int = INT_MAX;
        ending_row_int = INT_MAX;
    }

    // Validate and interpret column bounds
    if (strcmp(starting_column, "full") != 0) {
        starting_column_int = atoi(starting_column);
        ending_column_int = atoi(ending_column);
        // printf("Columns from %d to %d\n", starting_column_int, ending_column_int);
    }
    else {
        // printf("Full column\n");
        starting_column_int = INT_MAX;
        ending_column_int = INT_MAX;
    }

    // Print values array
    printf("\nProcessed Values:\n");
    for (int i = 0; i < values_size; i++) {
        printf("%s\n", values[i]);
        free(values[i]);
    }

    free(values);
    free(line);
    fclose(spreadsheet_fp);

    return 0;
}



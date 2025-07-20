#ifndef FAT_DATA_H
#define FAT_DATA_H

#include <stddef.h>

#define MAX_NUMBER_LENGTH 4096

/**
 * Checks if a string represents a valid floating-point number of arbitrary size.
 * Supports optional leading/trailing spaces, sign, decimal points, and exponents.
 *
 * @param str The string to check.
 * @return 1 if valid, 0 otherwise.
 */
int is_valid_double(const char *str);

/**
 * Compares two arbitrarily large non-negative integers represented as strings.
 *
 * @param num1 First number as a string.
 * @param num2 Second number as a string.
 * @return -1 if num1 < num2, 0 if equal, 1 if num1 > num2.
 */
int compare_big_numbers(const char *num1, const char *num2);

/**
 * Returns true if the number has a leading minus sign.
 *
 * @param num Number string.
 * @return 1 if negative, 0 otherwise.
 */
int is_negative(const char *num);

/**
 * Removes leading '+' or '-' from the number string.
 *
 * @param num Input number string.
 * @param out Output buffer (should be at least MAX_NUMBER_LENGTH).
 */
void strip_sign(const char *num, char *out);

/**
 * Adds two non-negative integer strings (no signs).
 *
 * @param num1 First number.
 * @param num2 Second number.
 * @param result Output buffer (must be large enough).
 */
void add_unsigned(const char *num1, const char *num2, char *result);

/**
 * Subtracts two non-negative integer strings (num1 >= num2).
 *
 * @param num1 First number (must be >= num2).
 * @param num2 Second number.
 * @param result Output buffer (must be large enough).
 */
void subtract_unsigned(const char *num1, const char *num2, char *result);

/**
 * Adds two arbitrarily large signed integers represented as strings.
 *
 * @param num1 First number.
 * @param num2 Second number.
 * @param result Output buffer (must be large enough).
 */
void add_big_integers(const char *num1, const char *num2, char *result);

/**
 * Subtracts two arbitrarily large signed integers represented as strings.
 *
 * @param num1 First number.
 * @param num2 Second number.
 * @param result Output buffer (must be large enough).
 */
void subtract_big_integers(const char *num1, const char *num2, char *result);

/**
 * Shifts a number left by appending zeros (multiplies by 10^digits).
 *
 * @param num Input number.
 * @param digits Number of zeros to append.
 * @param result Output buffer (must be large enough).
 */
void shift_left(const char *num, int digits, char *result);

/**
 * Pads a number with leading zeros to match the given total length.
 *
 * @param src Input number string.
 * @param total_length Desired total length.
 * @param out Output buffer.
 */
void pad_left_zeros(const char *src, int total_length, char *out);

/**
 * Multiplies two arbitrarily large integers using the Karatsuba algorithm.
 *
 * @param num1 First number.
 * @param num2 Second number.
 * @param result Output buffer (must be large enough).
 */
void karatsuba_multiply(const char *num1, const char *num2, char *result);

/**
 * Divides two big integers as strings and returns a string with decimal places.
 * 
 * @param num1 Dividend.
 * @param num2 Divisor.
 * @param precision Number of digits after the decimal point.
 * @param result Output buffer for the result.
 */
void divide_big_decimals(const char *num1, const char *num2, int precision, char *result);




#endif // FAT_DATA_H

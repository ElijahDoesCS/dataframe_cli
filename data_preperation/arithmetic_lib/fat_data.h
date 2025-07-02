#ifndef FAT_DATA_H
#define FAT_DATA_H

#include <stddef.h>

/**
 * Checks if a string represents a valid floating-point number of arbitrary size.
 * Supports optional leading/trailing spaces, sign, decimal points, and exponents.
 *
 * @param str The string to check.
 * @return 1 if valid, 0 otherwise.
 */
int isValidDouble(const char *str);

/**
 * Trims leading and trailing spaces from a string (in-place).
 *
 * @param str The string to trim.
 */
void trimSpaces(char *str);

/**
 * Compares two arbitrarily large numbers represented as strings.
 * Assumes both strings are valid numerical representations.
 *
 * @param num1 First number as a string.
 * @param num2 Second number as a string.
 * @return -1 if num1 < num2, 0 if equal, 1 if num1 > num2.
 */
int compareBigNumbers(const char *num1, const char *num2);

/**
 * Adds two arbitrarily large integers represented as strings.
 * 
 * @param num1 First number as a string.
 * @param num2 Second number as a string.
 * @param result Buffer to store the result (must be large enough).
 */
void addBigIntegers(const char *num1, const char *num2, char *result);

#endif // FAT_DATA_H

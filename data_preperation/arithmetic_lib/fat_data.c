#include "fat_data.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Tests whether a string is a valid floating-point number of arbitrary size
// Supports optional leading/trailing spaces, sign, decimal points, and exponents
int isValidDouble(const char *str) {
    if (!str || !*str) return 0;

    int seenDigit = 0, seenDot = 0, seenExp = 0;

    while (isspace((unsigned char)*str)) str++;

    if (*str == '+' || *str == '-') str++;

    for (; *str; str++) {
        if (isdigit((unsigned char)*str)) {
            seenDigit = 1;
        } else if (*str == '.') {
            if (seenDot || seenExp) return 0;
            seenDot = 1;
        } else if (*str == 'e' || *str == 'E') {
            if (seenExp || !seenDigit) return 0;
            seenExp = 1;
            seenDigit = 0;
            if (*(str + 1) == '+' || *(str + 1) == '-') str++;
        } else if (isspace((unsigned char)*str)) {
            break;
        } else {
            return 0;
        }
    }

    while (isspace((unsigned char)*str)) str++;

    return seenDigit && *str == '\0';
}

int compareBigNumbers(const char *num1, const char *num2) {
    while (*num1 == '0') num1++;  
    while (*num2 == '0') num2++;  

    size_t len1 = strlen(num1), len2 = strlen(num2);
    if (len1 > len2) return 1;
    if (len1 < len2) return -1;
    return strcmp(num1, num2);
}

void addBigIntegers(const char *num1, const char *num2, char *result) {
    int len1 = strlen(num1), len2 = strlen(num2);
    int carry = 0, sum, i, j, k = 0;
    
    char temp[1024] = {0};

    for (i = len1 - 1, j = len2 - 1; i >= 0 || j >= 0 || carry; i--, j--, k++) {
        int digit1 = (i >= 0) ? num1[i] - '0' : 0;
        int digit2 = (j >= 0) ? num2[j] - '0' : 0;
        sum = digit1 + digit2 + carry;
        carry = sum / 10;
        temp[k] = (sum % 10) + '0';
    }

    for (i = 0; i < k; i++) result[i] = temp[k - i - 1];
    result[k] = '\0';
}

void subtractBigIntegers(const char *num1, const char *num2, char *result) {
    
}

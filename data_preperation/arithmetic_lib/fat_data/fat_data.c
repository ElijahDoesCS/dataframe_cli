#include "fat_data.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int is_valid_double(const char *str) {
    if (!str || !*str) return 0;

    int seen_digit = 0, seen_dot = 0, seen_exp = 0;

    while (isspace((unsigned char)*str)) str++;

    if (*str == '+' || *str == '-') str++;

    for (; *str; str++) {
        if (isdigit((unsigned char)*str)) {
            seen_digit = 1;
        } else if (*str == '.') {
            if (seen_dot || seen_exp) return 0;
            seen_dot = 1;
        } else if (*str == 'e' || *str == 'E') {
            if (seen_exp || !seen_digit) return 0;
            seen_exp = 1;
            seen_digit = 0;
            if (*(str + 1) == '+' || *(str + 1) == '-') str++;
        } else if (isspace((unsigned char)*str)) {
            break;
        } else {
            return 0;
        }
    }

    while (isspace((unsigned char)*str)) str++;

    return seen_digit && *str == '\0';
}

int compare_big_numbers(const char *num1, const char *num2) {
    while (*num1 == '0') num1++;
    while (*num2 == '0') num2++;

    size_t len1 = strlen(num1), len2 = strlen(num2);
    if (len1 > len2) return 1;
    if (len1 < len2) return -1;

    int cmp = strcmp(num1, num2);
    if (cmp > 0) return 1;
    if (cmp < 0) return -1;
    return 0;
}

int is_negative(const char *num) {
    return num[0] == '-';
}

void strip_sign(const char *num, char *out) {
    if (num[0] == '-' || num[0] == '+')
        strcpy(out, num + 1);
    else
        strcpy(out, num);
}

void add_unsigned(const char *num1, const char *num2, char *result) {
    int len1 = strlen(num1), len2 = strlen(num2);
    int carry = 0, sum, i, j, k = 0;
    char temp[MAX_NUMBER_LENGTH] = {0};

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

void subtract_unsigned(const char *num1, const char *num2, char *result) {
    int len1 = strlen(num1), len2 = strlen(num2);
    int borrow = 0, diff, i, j, k = 0;
    char temp[MAX_NUMBER_LENGTH] = {0};

    for (i = len1 - 1, j = len2 - 1; i >= 0; i--, j--, k++) {
        int digit1 = num1[i] - '0' - borrow;
        int digit2 = (j >= 0) ? num2[j] - '0' : 0;
        borrow = 0;
        if (digit1 < digit2) {
            digit1 += 10;
            borrow = 1;
        }
        temp[k] = (digit1 - digit2) + '0';
    }

    while (k > 1 && temp[k - 1] == '0') k--;

    for (i = 0; i < k; i++) result[i] = temp[k - i - 1];
    result[k] = '\0';
}

void add_big_integers(const char *num1, const char *num2, char *result) {
    char a[MAX_NUMBER_LENGTH], b[MAX_NUMBER_LENGTH];
    int neg1 = is_negative(num1), neg2 = is_negative(num2);
    strip_sign(num1, a);
    strip_sign(num2, b);

    if (!neg1 && !neg2) {
        add_unsigned(a, b, result);
    } else if (neg1 && neg2) {
        add_unsigned(a, b, result);
        char tmp[MAX_NUMBER_LENGTH];
        strcpy(tmp, result);
        snprintf(result, MAX_NUMBER_LENGTH, "-%s", tmp);
    } else if (neg1 && !neg2) {
        if (compare_big_numbers(a, b) > 0) {
            subtract_unsigned(a, b, result);
            char tmp[MAX_NUMBER_LENGTH];
            strcpy(tmp, result);
            snprintf(result, MAX_NUMBER_LENGTH, "-%s", tmp);
        } else {
            subtract_unsigned(b, a, result);
        }
    } else {
        if (compare_big_numbers(a, b) >= 0) {
            subtract_unsigned(a, b, result);
        } else {
            subtract_unsigned(b, a, result);
            char tmp[MAX_NUMBER_LENGTH];
            strcpy(tmp, result);
            snprintf(result, MAX_NUMBER_LENGTH, "-%s", tmp);
        }
    }
}

void subtract_big_integers(const char *num1, const char *num2, char *result) {
    char negated[MAX_NUMBER_LENGTH];
    if (is_negative(num2)) {
        strip_sign(num2, negated);
    } else {
        snprintf(negated, MAX_NUMBER_LENGTH, "-%s", num2);
    }
    add_big_integers(num1, negated, result);
}

void pad_left_zeros(const char *src, int total_length, char *out) {
    int len = strlen(src);
    int padding = total_length - len;
    for (int i = 0; i < padding; i++) out[i] = '0';
    strcpy(out + padding, src);
}

void shift_left(const char *num, int digits, char *result) {
    size_t len = strlen(num);
    strcpy(result, num);
    for (int i = 0; i < digits; i++) {
        result[len + i] = '0';
    }
    result[len + digits] = '\0';
}

void strip_leading_zeros(char *num) {
    int i = 0;
    int is_neg = 0;

    if (num[0] == '-') {
        is_neg = 1;
        i = 1;
    }

    // Skip all leading zeros
    while (num[i] == '0') i++;

    if (num[i] == '\0') {
        // Result is all zeros
        strcpy(num, "0");
    } else if (is_neg) {
        memmove(num + 1, num + i, strlen(num + i) + 1); // shift content right after '-'
        num[0] = '-';
    } else {
        memmove(num, num + i, strlen(num + i) + 1);
    }
}

void karatsuba_multiply(const char *num1, const char *num2, char *result) {
    char a[MAX_NUMBER_LENGTH], b[MAX_NUMBER_LENGTH];
    strip_sign(num1, a);
    strip_sign(num2, b);
    int neg = is_negative(num1) ^ is_negative(num2);

    if (strlen(a) == 1 && strlen(b) == 1) {
        int product = (a[0] - '0') * (b[0] - '0');
        snprintf(result, MAX_NUMBER_LENGTH, "%d", product);
        if (neg && product != 0) {
            char temp[MAX_NUMBER_LENGTH];
            snprintf(temp, MAX_NUMBER_LENGTH, "-%s", result);
            strcpy(result, temp);
        }
        return;
    }

    int n = strlen(a) > strlen(b) ? strlen(a) : strlen(b);
    if (n % 2 != 0) n++;

    char a_padded[MAX_NUMBER_LENGTH], b_padded[MAX_NUMBER_LENGTH];
    pad_left_zeros(a, n, a_padded);
    pad_left_zeros(b, n, b_padded);

    int m = n / 2;

    char a1[MAX_NUMBER_LENGTH], a0[MAX_NUMBER_LENGTH];
    char b1[MAX_NUMBER_LENGTH], b0[MAX_NUMBER_LENGTH];
    strncpy(a1, a_padded, m); a1[m] = '\0';
    strcpy(a0, a_padded + m);
    strncpy(b1, b_padded, m); b1[m] = '\0';
    strcpy(b0, b_padded + m);

    char z2[MAX_NUMBER_LENGTH * 2]; karatsuba_multiply(a1, b1, z2);
    char z0[MAX_NUMBER_LENGTH * 2]; karatsuba_multiply(a0, b0, z0);

    char a1_plus_a0[MAX_NUMBER_LENGTH]; add_big_integers(a1, a0, a1_plus_a0);
    char b1_plus_b0[MAX_NUMBER_LENGTH]; add_big_integers(b1, b0, b1_plus_b0);
    char z1[MAX_NUMBER_LENGTH * 2]; karatsuba_multiply(a1_plus_a0, b1_plus_b0, z1);

    char temp[MAX_NUMBER_LENGTH * 2];
    subtract_big_integers(z1, z2, temp);
    subtract_big_integers(temp, z0, z1);

    char z2_shifted[MAX_NUMBER_LENGTH * 2]; shift_left(z2, 2 * (n - m), z2_shifted);
    char z1_shifted[MAX_NUMBER_LENGTH * 2]; shift_left(z1, n - m, z1_shifted);
    char temp1[MAX_NUMBER_LENGTH * 2]; add_big_integers(z2_shifted, z1_shifted, temp1);
    add_big_integers(temp1, z0, result);

    if (neg && strcmp(result, "0") != 0) {
        char tmp[MAX_NUMBER_LENGTH * 2];
        snprintf(tmp, MAX_NUMBER_LENGTH * 2, "-%s", result);
        strcpy(result, tmp);
    }

    // Remove leading zeros
    strip_leading_zeros(result);
}

void divide_big_decimals(const char *num1, const char *num2, int precision, char *result) {
    char dividend[MAX_NUMBER_LENGTH * 2];
    char divisor[MAX_NUMBER_LENGTH];
    char current[MAX_NUMBER_LENGTH * 2] = "0";
    char quotient[MAX_NUMBER_LENGTH * 4] = "";
    char sub_result[MAX_NUMBER_LENGTH * 2];
    char remainder[MAX_NUMBER_LENGTH * 2] = "";
    
    int neg = is_negative(num1) ^ is_negative(num2);
    strip_sign(num1, dividend);
    strip_sign(num2, divisor);

    if (strcmp(divisor, "0") == 0) {
        strcpy(result, "NaN");
        return;
    }

    if (strcmp(dividend, "0") == 0) {
        snprintf(result, MAX_NUMBER_LENGTH * 2, "0.%0*d", precision, 0);
        return;
    }

    size_t len = strlen(dividend);
    int i = 0, q_index = 0;

    while (i < len) {
        size_t current_len = strlen(current);
        current[current_len] = dividend[i++];
        current[current_len + 1] = '\0';
        strip_leading_zeros(current);

        int count = 0;
        while (compare_big_numbers(current, divisor) >= 0) {
            subtract_unsigned(current, divisor, sub_result);
            strcpy(current, sub_result);
            strip_leading_zeros(current);
            count++;
        }
        quotient[q_index++] = count + '0';
    }

    // Decimal point
    quotient[q_index++] = '.';

    int digits_generated = 0;
    while (digits_generated < precision) {
        if (strcmp(current, "0") == 0) {
            quotient[q_index++] = '0';
            digits_generated++;
            continue;
        }

        // Multiply current remainder by 10
        strcat(current, "0");

        // Perform division again
        int count = 0;
        while (compare_big_numbers(current, divisor) >= 0) {
            subtract_unsigned(current, divisor, sub_result);
            strcpy(current, sub_result);
            strip_leading_zeros(current);
            count++;
        }

        quotient[q_index++] = count + '0';
        digits_generated++;
    }

    quotient[q_index] = '\0';
    strip_leading_zeros(quotient);

    if (neg && strcmp(quotient, "0.000000") != 0) {
        snprintf(result, MAX_NUMBER_LENGTH * 4, "-%s", quotient);
    } else {
        strcpy(result, quotient);
    }
}

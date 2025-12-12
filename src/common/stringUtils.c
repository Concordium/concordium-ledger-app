#include "stringUtils.h"

/**
 * @brief Find the first occurrence of a character in a string
 *
 * Custom implementation of strchr to avoid standard library dependency.
 * Provides robust null pointer checking.
 */
const char* find_char(const char* str, char c) {
    if (!str) return NULL;

    while (*str) {
        if (*str == c) {
            return str;
        }
        str++;
    }
    return NULL;
}

/**
 * @brief Find the first occurrence of a substring in a string
 *
 * Custom implementation of strstr to avoid standard library dependency.
 * This function provides robust null pointer checking and consistent behavior.
 *
 * Reasons for custom implementation:
 * - Avoid potential security vulnerabilities in standard library implementations
 * - Ensure consistent behavior across different platforms
 * - Reduce dependencies on external libraries
 * - Better control over memory usage and performance
 * - Compatibility with Ledger's security model
 */
const char* find_substring(const char* haystack, const char* needle) {
    if (!haystack || !needle || !*needle) {
        return NULL;
    }

    const char* h = haystack;
    const char* n = needle;

    while (*h) {
        const char* h_start = h;
        const char* n_current = n;

        // Try to match the needle starting at current position
        while (*h && *n_current && *h == *n_current) {
            h++;
            n_current++;
        }

        // If we reached the end of needle, we found a match
        if (!*n_current) {
            return h_start;
        }

        // Reset and try next position
        h = h_start + 1;
    }

    return NULL;
}

/**
 * @brief Find the matching closing delimiter for a given opening delimiter
 *
 * This function handles nested delimiters by tracking depth levels.
 * Useful for parsing structured data with nested brackets, parentheses, etc.
 *
 * @param start Pointer to the character after the opening delimiter
 * @param open_char The opening delimiter character
 * @param close_char The closing delimiter character
 * @return Pointer to the matching closing delimiter, or NULL if not found
 */
char* find_matching_delimiter(char* start, char open_char, char close_char) {
    int depth = 1;
    char* current = start + 1;  // Skip the opening delimiter

    while (*current && depth > 0) {
        if (*current == open_char) {
            depth++;
        } else if (*current == close_char) {
            depth--;
        }
        current++;
    }

    if (depth == 0) {
        return current - 1;
    }
    return NULL;
}

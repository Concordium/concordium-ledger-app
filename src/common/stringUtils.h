#pragma once

#include <stddef.h>

/**
 * @file stringUtils.h
 * @brief Custom string utility functions to replace standard library functions
 *
 * This module provides custom implementations of string functions to avoid
 * dependencies on standard library functions that may not be available or
 * may have different behavior in the embedded Ledger environment.
 *
 * Reasons for custom implementations:
 * - Avoid potential security vulnerabilities in standard library implementations
 * - Ensure consistent behavior across different platforms
 * - Reduce dependencies on external libraries
 * - Better control over memory usage and performance
 * - Compatibility with Ledger's security model
 */

/**
 * @brief Find the first occurrence of a character in a string
 *
 * Custom implementation of strchr to avoid standard library dependency.
 *
 * @param str The string to search in (must not be NULL)
 * @param c The character to search for
 * @return Pointer to the first occurrence of c, or NULL if not found
 */
const char* find_char(const char* str, char c);

/**
 * @brief Find the first occurrence of a substring in a string
 *
 * Custom implementation of strstr to avoid standard library dependency.
 * This function provides robust null pointer checking and consistent behavior.
 *
 * @param haystack The string to search in (must not be NULL)
 * @param needle The substring to search for (must not be NULL and not empty)
 * @return Pointer to the first occurrence of needle, or NULL if not found
 */
const char* find_substring(const char* haystack, const char* needle);

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
char* find_matching_delimiter(char* start, char open_char, char close_char);

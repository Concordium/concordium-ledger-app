
#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "globals.h"

// Buffer size constants for CBOR string parsing
#define CBOR_NUMBER_FORMAT_SIZE  30   // Buffer size for number formatting
#define CBOR_TAG_NUMBER_SIZE     22   // Buffer size for tag number string
#define CBOR_COININFO_SIZE       16   // Buffer size for coin info display
#define CBOR_ADDRESS_BYTES_SIZE  32   // Buffer size for address bytes
#define CBOR_BASE58_ADDRESS_SIZE 57   // Buffer size for Base58 address
#define CBOR_MANTISSA_SIZE       258  // Buffer size for mantissa string
#define CBOR_EXPONENT_SIZE       32   // Buffer size for exponent string
#define CBOR_TAG_PATTERN_SIZE    32   // Buffer size for tag pattern matching

/*******************************************************************************
 * DATA STRUCTURES
 ******************************************************************************/

/**
 * @struct tag_info_t
 * @brief Structure representing a single parsed CBOR tag and its metadata.
 *
 * This structure holds all relevant information about a CBOR tag extracted from a buffer,
 * including its numeric identifier, the raw content (with delimiters), the length of the content,
 * the position where the tag starts in the original buffer, and a validity flag.
 *
 * Fields:
 * - tag_number: The numeric identifier of the CBOR tag (e.g., 121 for Tag(121)).
 * - content: Buffer containing the raw content of the tag, including delimiters (brackets/braces).
 *            Always null-terminated for safe string operations.
 * - content_length: The actual length of the content in bytes, not including the null terminator.
 *                   Used for precise content handling and memory operations.
 * - is_valid: Flag indicating whether this tag_info_t structure contains valid data.
 *             Used to distinguish between initialized and uninitialized entries.
 */
typedef struct {
    uint64_t tag_number;                 ///< CBOR tag identifier number
    char content[MAX_TAG_CONTENT_SIZE];  ///< Tag content with delimiters
    size_t content_length;               ///< Actual content length in bytes
    bool is_valid;                       ///< Validity flag for this entry
    char parsedContent[MAX_TAG_PARSED_CONTENT_SIZE];

} tag_info_t;

/**
 * @struct tag_list_t
 * @brief Container for multiple CBOR tags with count tracking
 *
 * This structure manages a collection of parsed CBOR tags, providing
 * both storage and metadata about the collection size.
 *
 * @var tag_list_t::tags
 * Array of tag_info_t structures holding the parsed tags.
 * Only the first 'count' entries contain valid data.
 *
 * @var tag_list_t::count
 * Number of valid tags currently stored in the tags array.
 * Always <= MAX_TAGS. Use this for safe iteration.
 */
typedef struct {
    tag_info_t tags[MAX_TAGS];  ///< Array of parsed tag information
    size_t count;               ///< Number of valid tags in array
} tag_list_t;

/*******************************************************************************
 * PUBLIC FUNCTION DECLARATIONS
 ******************************************************************************/

/**
 * @brief Extract CBOR tags from a string input (Ledger-compatible implementation)
 *
 * This function parses a string containing CBOR tag representations and extracts
 * them into a structured format. It handles nested delimiters and validates
 * content sizes to ensure memory safety. The implementation uses custom string
 * utilities to avoid dependencies on standard library functions.
 *
 * @details
 * The function searches for patterns like "Tag(N): content" where:
 * - N is a positive integer (tag number)
 * - content is delimited by [] (arrays) or {} (objects)
 * - Nested delimiters are properly handled using bracket matching
 *
 * The parsing process:
 * 1. Scans input string for "Tag(" patterns using custom substring search
 * 2. Extracts and validates tag numbers using custom number parsing
 * 3. Locates content boundaries using delimiter matching
 * 4. Validates content size constraints
 * 5. Stores extracted information in tag_list structure
 *
 * @param[in] input Null-terminated string containing CBOR tag representations.
 *                  Must not be NULL. Can contain multiple tags.
 * @param[out] tag_list Pointer to tag_list_t structure to populate with results.
 *                      Must not be NULL. Will be initialized by this function.
 *
 * @return true if parsing completed successfully (even if no tags found)
 * @return false if input validation failed or critical errors occurred
 *
 * @note The function will continue processing even if individual tags fail
 *       validation, allowing partial extraction of valid tags.
 *
 * @warning Input string must be null-terminated. Unterminated strings may
 *          cause undefined behavior.
 *
 * @example
 * @code
 * const char* cbor_string = "Tag(121): [1, 2, 3] Tag(122): {\"key\": \"value\"}";
 * tag_list_t extracted_tags;
 *
 * if (extract_tags_ledger(cbor_string, &extracted_tags)) {
 *     // Process extracted_tags.count tags
 *     for (size_t i = 0; i < extracted_tags.count; i++) {
 *         printf("Tag %llu: %.*s\n",
 *                extracted_tags.tags[i].tag_number,
 *                (int)extracted_tags.tags[i].content_length,
 *                extracted_tags.tags[i].content);
 *     }
 * }
 * @endcode
 */
bool extract_tags_ledger(const char* input, tag_list_t* tag_list);

/**
 * @brief Find a specific tag by its numeric identifier
 *
 * This function performs a linear search through the tag list to locate
 * a tag with the specified numeric identifier. It returns a pointer to
 * the tag_info_t structure for direct access to tag data.
 *
 * @param[in] tag_list Pointer to initialized tag_list_t structure to search.
 *                     Must not be NULL and should contain valid data.
 * @param[in] tag_number The numeric identifier of the tag to find.
 *                       Must match exactly with stored tag numbers.
 *
 * @return Pointer to tag_info_t structure if tag is found and valid
 * @return NULL if tag is not found, tag_list is NULL, or tag is invalid
 *
 * @note The returned pointer is valid only as long as the tag_list remains
 *       unchanged. Do not store the pointer across function calls that might
 *       modify the tag_list.
 *
 * @warning Do not modify the returned structure directly unless you understand
 *          the implications for data consistency.
 *
 * @example
 * @code
 * tag_list_t tag_list;
 * // ... populate tag_list ...
 *
 * tag_info_t* specific_tag = find_tag_by_number(&tag_list, 121);
 * if (specific_tag != NULL) {
 *     printf("Found tag 121 with content: %.*s\n",
 *            (int)specific_tag->content_length,
 *            specific_tag->content);
 * }
 * @endcode
 */
tag_info_t* find_tag_by_number(tag_list_t* tag_list, uint64_t tag_number);

/**
 * @brief Print extracted tags to debug output (using PRINTF)
 *
 * This function provides a formatted display of all valid tags in the tag list
 * for debugging and diagnostic purposes. It uses Ledger's PRINTF macro for
 * output, which may be redirected based on build configuration.
 *
 * @details
 * For each valid tag, the function displays:
 * - Sequential tag number (1-based indexing for display)
 * - CBOR tag identifier number
 * - Complete tag content with proper length handling
 * - Content length in bytes
 *
 * @param[in] tag_list Pointer to tag_list_t structure to display.
 *                     NULL input is handled gracefully (no output).
 *
 * @note This function is primarily intended for debugging and may not be
 *       available in production builds depending on PRINTF configuration.
 *
 * @note Output format is designed for readability in debug logs and may
 *       include multiple lines per tag.
 *
 * @example
 * @code
 * tag_list_t tag_list;
 * extract_tags_ledger(input_string, &tag_list);
 * print_tags_ledger(&tag_list);  // Outputs formatted tag information
 * @endcode
 */
void print_tags_ledger(const tag_list_t* tag_list);

/**
 * @brief Integration function for parsing tags from Ledger buffer_t structure
 *
 * This function provides seamless integration with Ledger's standard buffer_t
 * data structure, automatically handling null termination and buffer validation
 * before delegating to the core parsing functionality.
 *
 * @details
 * The function performs the following operations:
 * 1. Validates input parameters for NULL values
 * 2. Ensures proper null termination of buffer content
 * 3. Checks for buffer overflow conditions
 * 4. Delegates to extract_tags_ledger for actual parsing
 *
 * This is the recommended entry point when working with data received
 * through Ledger's communication protocols (APDU commands, etc.).
 *
 * @param[in] buffer Pointer to buffer_t structure containing CBOR data.
 *                   Must not be NULL. The buffer should contain text data
 *                   representing CBOR tags in string format.
 * @param[out] tag_list Pointer to tag_list_t structure to populate.
 *                      Must not be NULL. Will be initialized by this function.
 *
 * @return true if parsing completed successfully
 * @return false if buffer validation failed or parsing encountered critical errors
 *
 * @note The function modifies the buffer by adding null termination at the
 *       current offset position. Ensure buffer has sufficient capacity.
 *
 * @warning Buffer overflow protection is provided, but the caller should
 *          ensure buffer->offset represents valid data length.
 *
 * @example
 * @code
 * buffer_t received_data = {
 *     .ptr = data_from_apdu,
 *     .size = MAX_BUFFER_SIZE,
 *     .offset = actual_data_length
 * };
 *
 * tag_list_t parsed_tags;
 * if (parse_tags_in_buffer(&received_data, &parsed_tags)) {
 *     // Process parsed tags
 *     for (size_t i = 0; i < parsed_tags.count; i++) {
 *         // Handle each tag...
 *     }
 * }
 * @endcode
 */
bool parse_tags_in_buffer(buffer_t* buffer, tag_list_t* tag_list);

/*******************************************************************************
 * INTERNAL UTILITY FUNCTIONS (DOCUMENTED FOR COMPLETENESS)
 ******************************************************************************/

/*
 * Note: The following functions are static/internal utility functions used by
 * the public API. They are documented here for completeness but are not part
 * of the public interface. Their signatures may change in future versions.
 *
 * Internal functions include:
 * - find_matching_delimiter(): Matches nested delimiters (brackets/braces)
 * - find_substring(): Custom substring search replacing strstr()
 * - find_char(): Custom character search replacing strchr()
 * - parse_number(): Custom number parsing replacing strtoull()
 *
 * These functions were implemented to:
 * 1. Avoid dependencies on standard library string functions
 * 2. Provide precise control over memory access patterns
 * 3. Ensure compatibility with Ledger's embedded environment
 * 4. Enable custom error handling and validation
 */

/*******************************************************************************
 * USAGE GUIDELINES AND BEST PRACTICES
 ******************************************************************************/

/**
 * @section usage_guidelines Usage Guidelines
 *
 * @subsection memory_management Memory Management
 * - All structures use static allocation for predictable memory usage
 * - No dynamic memory allocation is performed
 * - Structures can be safely allocated on the stack
 * - No explicit cleanup is required
 *
 * @subsection error_handling Error Handling
 * - Functions return boolean success/failure indicators
 * - Invalid input is handled gracefully without crashes
 * - Partial parsing continues even if individual tags fail
 * - Use is_valid flag to check individual tag validity
 *
 * @subsection performance_considerations Performance Considerations
 * - Linear search for tag lookup (O(n) complexity)
 * - Parsing is single-pass for efficiency
 * - Memory copying is minimized where possible
 * - Custom string utilities avoid function call overhead
 * - Suitable for typical Ledger application tag counts
 *
 * @subsection security_notes Security Notes
 * - All buffer operations are bounds-checked
 * - Input validation prevents buffer overflows
 * - No external dependencies beyond standard types
 * - Custom string utilities provide controlled behavior
 * - Suitable for security-critical applications
 *
 * @subsection embedded_considerations Embedded Environment Notes
 * - No standard library string function dependencies
 * - Predictable memory usage patterns
 * - Optimized for constrained memory environments
 * - Compatible with Ledger's security model
 */

/**
 * @section example_usage Complete Example Usage
 *
 * @code
 * #include "cborParsing.h"
 *
 * void process_cbor_data(const char* cbor_string) {
 *     tag_list_t tags;
 *
 *     // Extract all tags from input
 *     if (!extract_tags_ledger(cbor_string, &tags)) {
 *         PRINTF("Failed to parse CBOR tags\n");
 *         return;
 *     }
 *
 *     PRINTF("Found %d tags\n", (int)tags.count);
 *
 *     // Look for specific tag
 *     tag_info_t* transfer_tag = find_tag_by_number(&tags, 121);
 *     if (transfer_tag != NULL) {
 *         PRINTF("Transfer data: %.*s\n",
 *                (int)transfer_tag->content_length,
 *                transfer_tag->content);
 *     }
 *
 *     // Process all tags
 *     for (size_t i = 0; i < tags.count; i++) {
 *         if (tags.tags[i].is_valid) {
 *             process_individual_tag(&tags.tags[i]);
 *         }
 *     }
 * }
 *
 * void process_buffer_data(buffer_t* buffer) {
 *     tag_list_t tags;
 *
 *     if (parse_tags_in_buffer(buffer, &tags)) {
 *         print_tags_ledger(&tags);  // Debug output
 *         // Process tags...
 *     }
 * }
 * @endcode
 */

/**
 * @section version_history Version History
 *
 * @subsection v1_0 Version 1.0
 * - Initial implementation with basic tag extraction
 * - Support for nested delimiter parsing
 * - Integration with Ledger buffer_t structure
 * - Custom string utilities for embedded compatibility
 * - Memory-safe bounded operations
 * - Comprehensive error handling and validation
 */

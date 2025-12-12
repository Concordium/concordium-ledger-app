#include "globals.h"
#include "ledger_assert.h"
#include "cborStrParsing.h"
#include "util.h"
#include "common/stringUtils.h"

typedef struct {
    union {
        int64_t signed_val;
        uint64_t unsigned_val;
    } value;
    bool is_signed;
    bool is_valid;  // Add validation flag
} parsed_number_t;

// Parse a number from string that can handle full uint64_t range
static parsed_number_t parse_number(const char* str, const char* end) {
    parsed_number_t result = {0};
    result.is_valid = false;  // Initialize as invalid

    uint64_t magnitude = 0;
    bool is_negative = false;
    const char* current = str;
    bool has_digits = false;

    // Skip whitespace
    while (current < end && (*current == ' ' || *current == '\t')) {
        current++;
    }

    // Check for negative sign
    if (current < end && *current == '-') {
        is_negative = true;
        current++;
    } else if (current < end && *current == '+') {
        current++;
    }

    // Parse digits
    while (current < end && *current >= '0' && *current <= '9') {
        has_digits = true;

        // Check for overflow before multiplication
        if (magnitude > (UINT64_MAX / 10)) {
            return result;  // Overflow would occur
        }

        uint64_t new_magnitude = magnitude * 10;

        // Check for overflow before addition
        if (new_magnitude > (UINT64_MAX - (*current - '0'))) {
            return result;  // Overflow would occur
        }

        magnitude = new_magnitude + (*current - '0');
        current++;
    }

    // Check if we have any digits
    if (!has_digits) {
        return result;  // No digits found
    }

    // Check if there are any non-whitespace characters after the number
    while (current < end && (*current == ' ' || *current == '\t')) {
        current++;
    }

    if (current < end) {
        return result;  // Invalid characters found after number
    }

    // Store the result based on sign with proper bounds checking
    if (is_negative) {
        // Check if magnitude fits in int64_t range
        if (magnitude > (uint64_t)INT64_MAX) {
            return result;  // Magnitude too large for int64_t
        }
        result.is_signed = true;
        result.value.signed_val = -(int64_t)magnitude;
    } else {
        result.is_signed = false;
        result.value.unsigned_val = magnitude;
    }

    result.is_valid = true;
    return result;
}

// Function to extract tag information from the input string (Ledger-compatible)
bool extract_tags_ledger(const char* input, tag_list_t* tag_list) {
    if (!input || !tag_list) return false;

    // Initialize tag list
    tag_list->count = 0;
    for (int i = 0; i < MAX_TAGS; i++) {
        tag_list->tags[i].is_valid = false;
        tag_list->tags[i].tag_number = 0;
        tag_list->tags[i].content_length = 0;
        memset(tag_list->tags[i].parsedContent, 0, sizeof(tag_list->tags[i].parsedContent));

        memset(tag_list->tags[i].content, 0, MAX_TAG_CONTENT_SIZE);
    }

    const char* current = input;

    while ((current = find_substring(current, "Tag(")) != NULL && tag_list->count < MAX_TAGS) {
        // Extract tag number
        const char* tag_start = current + 4;  // Skip "Tag("
        const char* tag_end = find_char(tag_start, ')');
        if (!tag_end) {
            PRINTF("Could not find closing parenthesis in tag\n");
            return false;
        }

        parsed_number_t num = parse_number(tag_start, tag_end);
        if (!num.is_valid) {
            PRINTF("Invalid tag number format\n");
            return false;
        }
        if (num.is_signed) {
            PRINTF("Tag number should be positive\n");
            return false;
        }
        // Parse tag number, tag is always positive so we use the unsigned value
        uint64_t tag_number = num.value.unsigned_val;

        // Find the colon after the tag
        const char* colon = find_char(tag_end, ':');
        if (!colon) {
            PRINTF("Could not find colon in tag\n");
            return false;
        }

        // Find the start of content (skip whitespace)
        const char* content_start = colon + 1;
        while (*content_start && (*content_start == ' ' || *content_start == '\t')) {
            content_start++;
        }

        if (!*content_start) {
            PRINTF("Could not find content in tag\n");
            return false;
        }

        // Determine content type and find matching delimiter
        const char* content_end = NULL;
        if (*content_start == '[') {
            content_end = find_matching_delimiter((char*)content_start, '[', ']');
        } else if (*content_start == '{') {
            content_end = find_matching_delimiter((char*)content_start, '{', '}');
        } else {
            content_end = find_char(content_start, ',');
        }

        if (!content_end) {
            PRINTF("Could not find content end in tag\n");
            return false;
        }

        // Calculate content length (inclusive of delimiters)
        size_t content_length = content_end - content_start + 1;

        // Check if content fits in our buffer
        if (content_length >= MAX_TAG_CONTENT_SIZE) {
            PRINTF("Tag content too large: %d bytes\n", (uint32_t)content_length);
            return false;
        }

        // Store tag information
        tag_list->tags[tag_list->count].tag_number =
            (uint64_t)tag_number;  // Cast to uint64_t since tags are positive
        tag_list->tags[tag_list->count].content_length = content_length;
        tag_list->tags[tag_list->count].is_valid = true;

        // Copy content to static buffer
        memcpy(tag_list->tags[tag_list->count].content, content_start, content_length);
        tag_list->tags[tag_list->count].content[content_length] = '\0';

        tag_list->count++;

        // Move past this tag for next iteration
        current = content_end + 1;
    }

    return true;
}

// Function to find a specific tag by number
tag_info_t* find_tag_by_number(tag_list_t* tag_list, uint64_t tag_number) {
    if (!tag_list) return NULL;

    for (size_t i = 0; i < tag_list->count; i++) {
        if (tag_list->tags[i].is_valid && tag_list->tags[i].tag_number == tag_number) {
            return &tag_list->tags[i];
        }
    }

    return NULL;
}

// Function to print extracted tags (using PRINTF)
void print_tags_ledger(const tag_list_t* tag_list) {
    if (!tag_list) return;
    char tag_number_str[CBOR_TAG_NUMBER_SIZE];
    PRINTF("Extracted %d tags:\n", (int)tag_list->count);
    for (size_t i = 0; i < tag_list->count; i++) {
        format_i64(tag_number_str, sizeof(tag_number_str), tag_list->tags[i].tag_number);
        if (tag_list->tags[i].is_valid) {
            PRINTF("Tag %d:\n", (int)(i + 1));
            PRINTF("  Number: %s\n", tag_number_str);
            PRINTF("  Content: %s\n", tag_list->tags[i].content);
            PRINTF("  Length: %d\n", (int)tag_list->tags[i].content_length);
        }
    }
}

bool parse_tag_40307(tag_info_t* tag) {
    PRINTF("about to parse tag 40307!\n");

    // Tag 40307 represents a tagged-holder-account
    // According to the schema it should contain a map with:
    // - Optional field 1: tagged-ccd-coininfo (tag 40305)
    // - Required field 3: 32 bytes representing a Concordium address
    if (!tag->is_valid) {
        PRINTF("Invalid tag\n");
        return false;
    }

    // The content format is: {Int:1,Tag(40305):{Int:1,Int:919,},Int:3,<HEX_STRING>}
    // We need to extract the coin info from Tag(40305) and the address from Int:3

    const char* content = tag->content;
    PRINTF("Tag 40307 content: %s\n", content);

    // Extract coin info from Tag(40305):{Int:1,Int:919,}
    const char* coininfo_start = find_substring(content, "Tag(40305):{Int:1,Int:");
    char coininfo[CBOR_COININFO_SIZE] = "none";  // Default value

    if (coininfo_start) {
        // Skip to the coin info number after "Tag(40305):{Int:1,Int:"
        const char* coininfo_num_start = coininfo_start + 22;  // Skip "Tag(40305):{Int:1,Int:"

        // Find the end of the coin info number (should be at ',' or '}')
        const char* coininfo_end = coininfo_num_start;
        while (*coininfo_end && *coininfo_end != ',' && *coininfo_end != '}' &&
               *coininfo_end != ' ') {
            coininfo_end++;
        }

        // Extract the coin info number
        size_t coininfo_length = coininfo_end - coininfo_num_start;
        if (coininfo_length > 0 && coininfo_length < sizeof(coininfo)) {
            memcpy(coininfo, coininfo_num_start, coininfo_length);
            coininfo[coininfo_length] = '\0';
        }
    }

    // Check if the coin info is valid (919 or none)
    if (strcmp(coininfo, "919") != 0 && strcmp(coininfo, "none") != 0) {
        PRINTF("Invalid coin info: %s\n", coininfo);
        THROW(ERROR_INVALID_COININFO);
    }

    // Look for "0x" in the content to find the address field
    const char* pos_0x = find_substring(content, "0x");

    if (!pos_0x) {
        PRINTF("Could not find 0x in tag 40307 content\n");
        return false;
    }

    // Skip past "0x" to find the hex value
    const char* hex_start = pos_0x + 2;  // Skip "0x"

    // Skip any whitespace
    while (*hex_start && (*hex_start == ' ' || *hex_start == '\t')) hex_start++;

    // Find the end of the hex string (should be at '}' or end of string)
    const char* hex_end = hex_start;
    while (*hex_end && *hex_end != '}' && *hex_end != ',' && *hex_end != ' ') {
        hex_end++;
    }

    // Calculate the length of the hex string
    size_t hex_length = hex_end - hex_start;

    if (hex_length != 64) {  // 32 bytes = 64 hex characters
        PRINTF("Invalid address length: expected 64 hex chars, got %d\n", (int)hex_length);
        return false;
    }

    // Validate that all characters are valid hex
    for (size_t i = 0; i < hex_length; i++) {
        char c = hex_start[i];
        if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))) {
            PRINTF("Invalid hex character at position %d: %c\n", (int)i, c);
            return false;
        }
    }

    // Convert hex string to bytes using utility function
    uint8_t address_bytes[CBOR_ADDRESS_BYTES_SIZE];
    if (!hex_string_to_bytes(hex_start, hex_length, address_bytes, sizeof(address_bytes))) {
        PRINTF("Failed to convert hex string to bytes\n");
        return false;
    }

    // Convert to base58 address format
    char base58_address[CBOR_BASE58_ADDRESS_SIZE];  // Base58 address needs ~57 chars
    size_t base58_length = sizeof(base58_address);

    if (base58check_encode(address_bytes,
                           sizeof(address_bytes),
                           (unsigned char*)base58_address,
                           &base58_length) == -1) {
        PRINTF("Failed to encode address as base58\n");
        return false;
    }
    // Hack for the address to be displayed correctly on the screen
    // Ensure we don't write beyond the buffer bounds
    if (base58_length > 50) {
        base58_address[50] = '\0';
    }

    // Format the output with base58 address (without coinInfo complexity)
    snprintf(tag->parsedContent, MAX_TAG_PARSED_CONTENT_SIZE, "\"%s\"", base58_address);

    PRINTF("Parsed tag: %s\n", tag->parsedContent);
    return true;
}

bool parse_tag_4(tag_info_t* tag) {
    PRINTF("about to parse tag 4!\n");

    // Tag 4 represents a decimal fraction [exponent, mantissa]
    // Content format: [Int:-2,Int:1223,]
    // Result: mantissa * 10^exponent = 1223 * 10^(-2) = 12.23

    if (!tag->is_valid) {
        PRINTF("Invalid tag\n");
        return false;
    }

    const char* content = tag->content;
    PRINTF("Tag 4 content: %s\n", content);

    // Extract the exponent (first Int)
    const char* first_int_pos = find_substring(content, "Int:");
    if (!first_int_pos) {
        PRINTF("Could not find first Int in tag 4 content\n");
        return false;
    }

    // Skip past "Int:" to get to the number
    const char* exponent_start = first_int_pos + 4;

    // Find the end of the first number (should be at ',')
    const char* exponent_end = exponent_start;
    while (*exponent_end && *exponent_end != ',') {
        exponent_end++;
    }

    // Parse the exponent, it is supposed to be negative
    parsed_number_t num = parse_number(exponent_start, exponent_end);
    if (!num.is_valid) {
        PRINTF("Invalid exponent format\n");
        return false;
    }
    if (!num.is_signed && num.value.unsigned_val != 0) {
        PRINTF("Warning: positive exponent %llu found\n",
               (unsigned long long)num.value.unsigned_val);
        return false;
    }
    int64_t exponent = num.value.signed_val;

    // Find the second Int (mantissa)
    const char* second_int_pos = find_substring(exponent_end, "Int:");
    if (!second_int_pos) {
        PRINTF("Could not find second Int in tag 4 content\n");
        return false;
    }

    // Skip past "Int:" to get to the number
    const char* mantissa_start = second_int_pos + 4;

    // Find the end of the second number (should be at ',' or ']')
    const char* mantissa_end = mantissa_start;
    while (*mantissa_end && *mantissa_end != ',' && *mantissa_end != ']') {
        mantissa_end++;
    }

    // Parse the mantissa, it is supposed to be positive
    num = parse_number(mantissa_start, mantissa_end);
    if (!num.is_valid) {
        PRINTF("Invalid mantissa format\n");
        return false;
    }
    if (num.is_signed) {
        PRINTF("Warning: negative mantissa %lld found\n", (long long)num.value.signed_val);
        return false;
    }
    uint64_t mantissa = num.value.unsigned_val;

    PRINTF("Parsed exponent: %lld, mantissa: %llu\n",
           (long long)exponent,
           (unsigned long long)mantissa);

    // Check for extreme values that would create unreadable output
    int64_t abs_exponent = -exponent;

    // Convert mantissa to string first to get its length
    char mantissa_str[CBOR_MANTISSA_SIZE];
    if (!format_u64(mantissa_str, sizeof(mantissa_str), mantissa)) {
        PRINTF("Failed to format mantissa\n");
        return false;
    }
    size_t mantissa_len = strlen(mantissa_str);

    // Handle extreme cases with scientific notation or sensible limits
    if (abs_exponent > 50) {
        // For very negative exponents, use scientific notation: mantissa * 10^exponent
        char exponent_str[CBOR_EXPONENT_SIZE];
        format_i64(exponent_str, sizeof(exponent_str), exponent);
        snprintf(tag->parsedContent,
                 MAX_TAG_PARSED_CONTENT_SIZE,
                 "%s * 10^%s",
                 mantissa_str,
                 exponent_str);
        PRINTF("Using scientific notation for extreme exponent\n");
    } else if (abs_exponent == 0) {
        // No decimal places needed
        snprintf(tag->parsedContent, MAX_TAG_PARSED_CONTENT_SIZE, "%s", mantissa_str);
    } else if (mantissa_len > (size_t)abs_exponent) {
        // Place decimal point within the string
        size_t int_len = mantissa_len - abs_exponent;
        if (int_len + 1 + abs_exponent + 1 > MAX_TAG_PARSED_CONTENT_SIZE) {
            PRINTF("Buffer too small for formatted decimal\n");
            return false;
        }
        memcpy(tag->parsedContent, mantissa_str, int_len);
        tag->parsedContent[int_len] = '.';
        memcpy(tag->parsedContent + int_len + 1, mantissa_str + int_len, abs_exponent);
        tag->parsedContent[int_len + 1 + abs_exponent] = '\0';
    } else {
        // Number is less than 1, need leading zeros after decimal
        size_t zeros = abs_exponent - mantissa_len;

        // Limit the number of leading zeros to keep output readable
        if (zeros > 15) {
            // Use scientific notation for very small numbers
            char exponent_str[CBOR_EXPONENT_SIZE];
            format_i64(exponent_str, sizeof(exponent_str), exponent);
            snprintf(tag->parsedContent,
                     MAX_TAG_PARSED_CONTENT_SIZE,
                     "%s * 10^%s",
                     mantissa_str,
                     exponent_str);
            PRINTF("Using scientific notation for very small number\n");
        } else {
            // Use normal decimal representation with limited zeros
            if (2 + zeros + mantissa_len + 1 > MAX_TAG_PARSED_CONTENT_SIZE) {
                PRINTF("Buffer too small for formatted decimal\n");
                return false;
            }
            char* out = tag->parsedContent;
            *out++ = '0';
            *out++ = '.';
            for (size_t i = 0; i < zeros; i++) {
                *out++ = '0';
            }
            memcpy(out, mantissa_str, mantissa_len);
            out[mantissa_len] = '\0';
        }
    }

    PRINTF("Parsed decimal: %s\n", tag->parsedContent);
    return true;
}

bool parse_tag_24(tag_info_t* tag) {
    PRINTF("about to parse tag 24!\n");

    // Tag 24 represents encoded CBOR data, often containing hex-encoded strings
    // Content format: ByteString(15): 0x6E5468697320697320612074657374,
    // Goal: Convert hex to human-readable characters

    if (!tag->is_valid) {
        PRINTF("Invalid tag\n");
        return false;
    }

    const char* content = tag->content;
    PRINTF("Tag 24 content: %s\n", content);

    // Convert hex to human-readable characters
    // First, find the hex string
    const char* pos_0x = find_substring(content, "0x");
    if (!pos_0x) {
        PRINTF("Could not find hex string in tag 24 content\n");
        return false;
    }

    // Skip past "0x" to get to the hex string
    const char* hex_start = pos_0x + 2;

    // Skip any whitespace
    while (*hex_start && (*hex_start == ' ' || *hex_start == '\t')) hex_start++;

    // Find the end of the hex string (should be at ',' or '}')
    const char* hex_end = hex_start;
    while (*hex_end && *hex_end != ',' && *hex_end != '}') {
        hex_end++;
    }

    // Calculate the length of the hex string
    size_t hex_length = hex_end - hex_start;

    // Convert hex string to human-readable characters
    if (!hex_string_to_ascii(hex_start,
                             hex_length,
                             tag->parsedContent,
                             sizeof(tag->parsedContent))) {
        PRINTF("Failed to convert hex string to ASCII\n");
        return false;
    }

    // Get the actual length of the converted ASCII string
    size_t ascii_length = strlen(tag->parsedContent);

    // Check bounds before writing comma and null terminator
    if (ascii_length + 2 >= sizeof(tag->parsedContent)) {
        PRINTF("Buffer too small for comma and null terminator\n");
        return false;
    }

    tag->parsedContent[ascii_length] = ',';
    tag->parsedContent[ascii_length + 1] = '\0';

    PRINTF("Parsed tag 24: %s\n", tag->parsedContent);
    return true;
}

bool interpret_tag(tag_info_t* tag) {
    switch (tag->tag_number) {
        // Tag 4 represents a decimal fraction [exponent, mantissa]
        // Content format: [Int:-2,Int:1223,]
        // Result: mantissa * 10^exponent = 1223 * 10^(-2) = 12.23
        case 4:
            return parse_tag_4(tag);
        // Tag 24 represents encoded CBOR data, often containing hex-encoded strings
        // Content format: ByteString(15): 0x6E5468697320697320612074657374
        // Goal: Convert hex to human-readable characters
        case 24:
            return parse_tag_24(tag);
        // Tag 40307 represents a tagged-holder-account
        // According to the schema it should contain a map with:
        // - Optional field 1: tagged-ccd-coininfo (tag 40305)
        // - Required field 3: 32 bytes representing a Concordium address
        case 40307:
            return parse_tag_40307(tag);
        default:
            PRINTF("Unhandled tag number: %llu\n", tag->tag_number);
            return false;
    }
}

bool replace_tag_with_parsed_content(buffer_t* buffer, const tag_info_t* tag) {
    if (!buffer || !buffer->ptr || !tag->is_valid || strlen(tag->parsedContent) == 0) {
        PRINTF("Invalid parameters for tag replacement\n");
        return false;
    }

    // Create the search pattern ",Tag(tag_number):"
    char tag_pattern[CBOR_TAG_PATTERN_SIZE];
    char tag_number_str[CBOR_TAG_NUMBER_SIZE];
    format_i64(tag_number_str, sizeof(tag_number_str), tag->tag_number);
    snprintf(tag_pattern, sizeof(tag_pattern), ",Tag(%s):", tag_number_str);

    PRINTF("Looking for pattern: '%s'\n", tag_pattern);
    PRINTF("Buffer content: %s\n", (char*)buffer->ptr);

    // Find the tag pattern in the buffer
    const char* tag_start = find_substring((const char*)buffer->ptr, tag_pattern);
    if (!tag_start) {
        PRINTF("Could not find tag pattern in buffer\n");
        return false;
    }

    PRINTF("Found tag at position: %d\n", (int)(tag_start - (const char*)buffer->ptr));

    // Calculate the total length to replace: ",Tag(N):" + content_length
    size_t pattern_length = strlen(tag_pattern);
    size_t total_replace_length = pattern_length + tag->content_length;

    // Get the position in the buffer where replacement starts
    size_t replace_start_pos = tag_start - (const char*)buffer->ptr;

    // Get lengths
    size_t buffer_length = strlen((const char*)buffer->ptr);
    size_t parsed_content_length = strlen(tag->parsedContent);

    // Add 1 for the colon prefix
    size_t replacement_content_length = parsed_content_length + 1;

    // Calculate new buffer length after replacement
    size_t new_buffer_length = buffer_length - total_replace_length + replacement_content_length;

    // CHECK FOR BUFFER OVERFLOW
    if (new_buffer_length >= buffer->size) {
        PRINTF("Buffer overflow: new length %d >= buffer capacity %d\n",
               (int)new_buffer_length,
               (int)buffer->size);
        return false;
    }

    PRINTF("Replace start pos: %d, total replace length: %d, replacement content length: %d\n",
           (int)replace_start_pos,
           (int)total_replace_length,
           (int)replacement_content_length);

    // Cast to char* for modification
    char* mutable_buffer = (char*)buffer->ptr;

    // If the new content is longer than what we're replacing, we need to move content
    if (replacement_content_length > total_replace_length) {
        // Move the content after the tag to make room
        size_t content_after_pos = replace_start_pos + total_replace_length;
        size_t content_after_length = buffer_length - content_after_pos;

        // Additional safety check
        if (replace_start_pos + replacement_content_length + content_after_length >= buffer->size) {
            PRINTF("Buffer overflow during content move\n");
            return false;
        }

        // Move content to the right
        memmove(mutable_buffer + replace_start_pos + replacement_content_length,
                mutable_buffer + content_after_pos,
                content_after_length + 1);  // +1 for null terminator
    }
    // If the new content is shorter, we need to move content left
    else if (replacement_content_length < total_replace_length) {
        // Move the content after the tag to close the gap
        size_t content_after_pos = replace_start_pos + total_replace_length;
        size_t content_after_length = buffer_length - content_after_pos;

        // Move content to the left
        memmove(mutable_buffer + replace_start_pos + replacement_content_length,
                mutable_buffer + content_after_pos,
                content_after_length + 1);  // +1 for null terminator
    }

    // Safety check before copying
    if (replace_start_pos + replacement_content_length > buffer->size) {
        PRINTF("Buffer overflow during content copy\n");
        return false;
    }

    // Copy the colon and parsed content into the buffer
    mutable_buffer[replace_start_pos] = ':';
    memcpy(mutable_buffer + replace_start_pos + 1, tag->parsedContent, parsed_content_length);

    // Update the buffer length if it changed
    if (replacement_content_length != total_replace_length) {
        mutable_buffer[new_buffer_length] = '\0';
    }

    PRINTF("Successfully replaced tag content. New buffer length: %d\n",
           (int)strlen((const char*)buffer->ptr));

    // Update buffer size to reflect new content length
    buffer->size = new_buffer_length;
    PRINTF("NEW BUFFER LENGTH: %d\n", new_buffer_length);

    return true;
}

bool remove_useless_commas(buffer_t* buffer) {
    if (!buffer || !buffer->ptr) {
        PRINTF("Invalid buffer for comma removal\n");
        return false;
    }

    char* mutable_buffer = (char*)buffer->ptr;
    size_t buffer_length = strlen(mutable_buffer);
    size_t write_pos = 0;

    PRINTF("Before cleanup: %s\n", mutable_buffer);

    for (size_t read_pos = 0; read_pos < buffer_length; read_pos++) {
        char current_char = mutable_buffer[read_pos];

        // Check if current character is a comma
        if (current_char == ',') {
            // Look ahead to see if the next non-whitespace character is a closing delimiter
            size_t next_pos = read_pos + 1;

            // Skip whitespace
            while (next_pos < buffer_length &&
                   (mutable_buffer[next_pos] == ' ' || mutable_buffer[next_pos] == '\t')) {
                next_pos++;
            }

            // Check if next character is a closing delimiter OR if we've reached the end
            if (next_pos >= buffer_length) {
                // Trailing comma at the end of buffer
                PRINTF("Removing trailing comma at position %d (end of buffer)\n", (int)read_pos);
                continue;
            } else if (mutable_buffer[next_pos] == '}' || mutable_buffer[next_pos] == ']') {
                // Comma before closing delimiter
                PRINTF("Removing comma at position %d before '%c'\n",
                       (int)read_pos,
                       mutable_buffer[next_pos]);
                continue;
            } else if (mutable_buffer[next_pos] == '{') {
                // Check if this comma should be a colon (JSON map key case)
                // Look backward to see if we have a quoted string just before this comma
                bool is_map_key = false;
                if (write_pos > 0 && mutable_buffer[write_pos - 1] == '"') {
                    // We have a quote just before the comma, look further back for opening quote
                    size_t quote_start = write_pos - 2;
                    while (quote_start > 0 && mutable_buffer[quote_start] != '"') {
                        quote_start--;
                    }

                    // Check if this looks like a quoted key followed by opening brace
                    if (quote_start > 0 && mutable_buffer[quote_start] == '"') {
                        is_map_key = true;
                        PRINTF("Converting comma to colon at position %d (JSON map key)\n",
                               (int)read_pos);
                    }
                }

                if (is_map_key) {
                    // Replace comma with colon for JSON map key syntax
                    mutable_buffer[write_pos] = ':';
                    write_pos++;
                    continue;
                }
            }
        }

        // Copy character to write position
        if (write_pos != read_pos) {
            mutable_buffer[write_pos] = current_char;
        }
        write_pos++;
    }

    // Null terminate at new length
    mutable_buffer[write_pos] = '\0';

    // Update buffer size
    buffer->size = write_pos;

    PRINTF("After cleanup: %s\n", mutable_buffer);
    PRINTF("New buffer length: %d\n", (int)write_pos);

    return true;
}

bool parse_tags_in_buffer(buffer_t* buffer, tag_list_t* tag_list) {
    if (!buffer || !buffer->ptr || !tag_list) {
        return false;
    }

    if (!extract_tags_ledger((const char*)buffer->ptr, tag_list)) {
        PRINTF("error while extracting tags\n");
        return false;
    };

    print_tags_ledger(tag_list);

    PRINTF("buffer before interpretation and replacement: %s\n", buffer->ptr);
    bool tag_interpretation_success = false;
    bool tag_replacement_success = false;
    for (size_t i = 0; i < tag_list->count; i++) {
        if (tag_list->tags[i].is_valid) {
            tag_interpretation_success = interpret_tag(&tag_list->tags[i]);
            tag_replacement_success = replace_tag_with_parsed_content(buffer, &tag_list->tags[i]);
        }
        if (!tag_interpretation_success || !tag_replacement_success) {
            PRINTF("Error while interpreting or replacing tags\n");
            return false;
        }
    }
    remove_useless_commas(buffer);

    PRINTF("FINAL BUFFER: %s.\n", buffer->ptr);
    return true;
}
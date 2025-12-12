#include "globals.h"
#include "read.h"
#include "io.h"  // io_send_sw
#include "cbor.h"
#include "ledger_assert.h"
#include "format.h"  // format_i64, format_hex
#include "cborStrParsing.h"
#include "cborinternal_p.h"
#include "common/stringUtils.h"

static signPLTContext_t* ctx = &global.withDataBlob.signPLTContext;
static tx_state_t* tx_state = &global_tx_state;

#define P1_INITIAL 0x01

static void indent(int nesting_level) {
    while (nesting_level--) PRINTF("  ");
}

bool cbor_read_string_or_byte_string(CborValue* it,
                                     char* output_ptr,
                                     size_t* output_size,
                                     size_t buffer_size,
                                     bool is_string) {
    if (is_string) {
        LEDGER_ASSERT(cbor_value_is_text_string(it), "expected string did not get it");
    } else {
        LEDGER_ASSERT(cbor_value_is_byte_string(it), "expected byte string did not get it");
    }

    const char* string_ptr;
    CborError err = _cbor_value_get_string_chunk(it, (const void**)&string_ptr, output_size, NULL);
    if (err) {
        return true;
    }

    // Check for buffer overflow
    if (*output_size > buffer_size) {
        PRINTF("Buffer overflow: string size %zu exceeds buffer size %zu\n",
               *output_size,
               buffer_size);
        return true;
    }

    // Copy the string data to the output buffer
    if (*output_size > 0 && output_ptr != NULL) {
        memcpy(output_ptr, string_ptr, *output_size);
    }

    return false;
}

void add_char_array_to_buffer(buffer_t* dst, char* src, size_t src_size) {
    if (dst->size - dst->offset < src_size) {
        PRINTF(
            "src_size: 0x%08X, "
            "dst->size-offset: "
            "0x%08X\n",
            src_size,
            dst->size - dst->offset);
        PRINTF("The destination buffer is too small\n");
        THROW(ERROR_PLT_BUFFER_ERROR);
    }
    memcpy((void*)(dst->ptr + dst->offset), src, src_size);
    dst->offset += src_size;
}

CborError decode_cbor_recursive(CborValue* it,
                                int nesting_level,
                                buffer_t* out_buf,
                                size_t buffer_size) {
    const char* temp;
    while (!cbor_value_at_end(it)) {
        CborError err;
        CborType type = cbor_value_get_type(it);

        // Check if we have enough space in the buffer before proceeding
        if (out_buf->offset >= buffer_size) {
            PRINTF("Buffer overflow: offset %zu >= buffer size %zu\n",
                   out_buf->offset,
                   buffer_size);
            return CborErrorInternalError;  // Return error to stop recursion
        }

        indent(nesting_level);
        switch (type) {
            case CborArrayType:
            case CborMapType: {
                // recursive type
                CborValue recursed;
                LEDGER_ASSERT(cbor_value_is_container(it), "Should be a container but isn't");
                if (type == CborArrayType) {
                    temp = "[";
                } else {
                    temp = "{";
                }
                PRINTF("%s", temp);
                add_char_array_to_buffer(out_buf, (char*)temp, strlen(temp));

                err = cbor_value_enter_container(it, &recursed);
                if (err) return err;  // parse error
                err = decode_cbor_recursive(&recursed, nesting_level + 1, out_buf, buffer_size);
                if (err) return err;  // parse error
                err = cbor_value_leave_container(it, &recursed);
                if (err) return err;  // parse error
                indent(nesting_level);
                if (type == CborArrayType) {
                    temp = "],";
                } else {
                    temp = "},";
                }
                PRINTF("%s", temp);
                add_char_array_to_buffer(out_buf, (char*)temp, strlen(temp));
                continue;
            }
            case CborIntegerType: {
                char integer_value[CBOR_INTEGER_BUFFER_SIZE];    // Buffer for uint64 integer
                                                                 // formatting
                char integer_display[CBOR_INTEGER_PREFIX_SIZE];  // Buffer for "Int:" prefix +
                                                                 // uint64 integer
                uint64_t raw_val = 0;

                // Get the raw integer value first
                if (cbor_value_get_raw_integer(it, &raw_val) != CborNoError) {
                    PRINTF("cbor_value_get_raw_integer error\n");
                }

                if (cbor_value_is_negative_integer(it)) {
                    // Handle negative integers
                    int64_t signed_val = -(int64_t)(raw_val + 1);
                    format_i64(integer_value, sizeof(integer_value), signed_val);
                } else {
                    // Handle positive integers
                    format_u64(integer_value, sizeof(integer_value), raw_val);
                }

                snprintf(integer_display, sizeof(integer_display), "Int:%s,", integer_value);
                add_char_array_to_buffer(out_buf, integer_display, strlen(integer_display));
                break;
            }

            case CborByteStringType: {
                uint8_t byte_string_data[CBOR_STRING_BUFFER_SIZE];
                size_t byte_string_length;
                err = cbor_read_string_or_byte_string(it,
                                                      (char*)byte_string_data,
                                                      &byte_string_length,
                                                      sizeof(byte_string_data),
                                                      false);
                if (err) return err;
                char string_value[CBOR_HEX_DISPLAY_SIZE] = {0};
                if (format_hex(byte_string_data,
                               byte_string_length,
                               string_value,
                               sizeof(string_value)) == -1) {
                    PRINTF("format_hex error\n");
                    THROW(ERROR_PLT_CBOR_ERROR);
                }
                add_char_array_to_buffer(out_buf, (char*)"0x", 2);
                add_char_array_to_buffer(out_buf, string_value, strlen(string_value));
                add_char_array_to_buffer(out_buf, (char*)",", 1);
                PRINTF("ByteString(%d): 0x%s\n", byte_string_length, string_value);
                break;
            }

            case CborTextStringType: {
                uint8_t text_string_data[CBOR_STRING_BUFFER_SIZE];
                size_t text_string_length;
                err = cbor_read_string_or_byte_string(it,
                                                      (char*)text_string_data,
                                                      &text_string_length,
                                                      sizeof(text_string_data),
                                                      true);
                if (err) return err;

                char text_display[CBOR_TEXT_DISPLAY_SIZE];

                // Handle null termination safely
                if (text_string_length < sizeof(text_string_data)) {
                    // Normal case: we have space for null terminator
                    text_string_data[text_string_length] = '\0';
                    snprintf(text_display, sizeof(text_display), "\"%s\",", text_string_data);
                } else {
                    // Edge case: string fills entire buffer, cannot null terminate
                    // Use snprintf with precision to limit the number of characters read
                    snprintf(text_display,
                             sizeof(text_display),
                             "\"%.*s\",",
                             (int)text_string_length,
                             text_string_data);
                }

                PRINTF("%s", text_display);
                add_char_array_to_buffer(out_buf, text_display, strlen(text_display));
                break;
            }

            case CborTagType: {
                CborTag tag;
                char tag_number[16];
                cbor_value_get_tag(it, &tag);  // can't fail
                format_u64(tag_number, sizeof(tag_number), tag);
                char tag_str[CBOR_TAG_STRING_SIZE];
                snprintf(tag_str, sizeof(tag_str), "Tag(%s):", tag_number);
                PRINTF("%s", tag_str);
                add_char_array_to_buffer(out_buf, tag_str, strlen(tag_str));

                break;
            }

            case CborSimpleType: {
                uint8_t simple_type_value;
                cbor_value_get_simple_type(it, &simple_type_value);  // can't fail
                PRINTF("simple(%d)\n", simple_type_value);
                break;
            }

            case CborNullType:
                temp = "null,";
                PRINTF("null");
                add_char_array_to_buffer(out_buf, (char*)temp, strlen(temp));
                break;

            case CborUndefinedType:
                temp = "undefined,";
                PRINTF("undefined");
                add_char_array_to_buffer(out_buf, (char*)temp, strlen(temp));
                break;

            case CborBooleanType: {
                bool val;
                cbor_value_get_boolean(it, &val);  // can't fail
                temp = val ? "true," : "false,";
                PRINTF(temp);
                add_char_array_to_buffer(out_buf, (char*)temp, strlen(temp));
                break;
            }

            case CborFloatType: {
                float float_value;
                char float_display[CBOR_FLOAT_DISPLAY_SIZE];
                cbor_value_get_float(it, &float_value);
                snprintf(float_display,
                         sizeof(float_display),
                         "Float:0x%08x,",
                         (uint32_t)float_value);
                PRINTF("Float: 0x%08x\n", (uint32_t)float_value);
                add_char_array_to_buffer(out_buf, float_display, strlen(float_display));
                break;
            }

            case CborDoubleType: {
                double val;
                char double_display[CBOR_FLOAT_DISPLAY_SIZE];
                cbor_value_get_double(it, &val);
                snprintf(double_display, sizeof(double_display), "Double:0x%08x,", (uint32_t)val);
                PRINTF("Double: 0x%08x\n", (uint32_t)val);
                add_char_array_to_buffer(out_buf, double_display, strlen(double_display));
                break;
            }

            case CborHalfFloatType: {
                uint16_t val;
                char temp2[16];
                cbor_value_get_half_float(it, &val);
                snprintf(temp2, sizeof(temp2), "__f16(%04x),", val);
                PRINTF("__f16(%04x)\n", val);
                add_char_array_to_buffer(out_buf, temp2, strlen(temp2));
                break;
            }

            case CborInvalidType:
                LEDGER_ASSERT(false, "Can't happen");  // can't happen
                break;
        }

        err = cbor_value_advance_fixed(it);
        if (err) return err;
    }
    return CborNoError;
}

bool parse_plt_cbor(uint8_t* cbor, size_t cbor_length) {
    CborParser parser;
    CborValue it;
    CborError err;

    // Initialize parser
    err = cbor_parser_init(cbor, cbor_length, 0, &parser, &it);
    if (err) {
        return false;
    }

    char temp[MAX_PLT_DIPLAY_STR] = {0};
    buffer_t out_buf = {.ptr = (const uint8_t*)temp, .size = MAX_PLT_DIPLAY_STR, .offset = 0};
    tag_list_t tag_list;  // initiate an empty tag_list_t
    err = decode_cbor_recursive(&it, 0, &out_buf, MAX_PLT_DIPLAY_STR);
    if (err) {
        PRINTF("Error while decoding cbor\n");
        THROW(ERROR_PLT_CBOR_ERROR);
    }

    if (!parse_tags_in_buffer(&out_buf, &tag_list)) {
        PRINTF("Error while parsing cbor tags\n");
        THROW(ERROR_PLT_CBOR_ERROR);
    }
    if (sizeof(ctx->pltOperationDisplay) < out_buf.size + 1) {
        PRINTF("display str is too small for value %d < %d\n",
               sizeof(ctx->pltOperationDisplay),
               out_buf.size);
        THROW(ERROR_PLT_BUFFER_ERROR);
    }
    memcpy(ctx->pltOperationDisplay, out_buf.ptr, out_buf.size);
    ctx->pltOperationDisplay[out_buf.size] = '\0';

    return true;
}

static bool extract_field_value(const char* input,
                                const char* field_name,
                                char* output,
                                size_t output_size) {
    // Look for pattern: "field_name":value
    char pattern[CBOR_PATTERN_BUFFER_SIZE];
    snprintf(pattern, sizeof(pattern), "\"%s\":", field_name);

    const char* field_pos = find_substring(input, pattern);
    if (!field_pos) return false;

    // Move to after the pattern
    const char* value_start = field_pos + strlen(pattern);

    // Skip whitespace
    while (*value_start == ' ') value_start++;

    const char* value_end;
    size_t copy_len;

    if (*value_start == '"') {
        // String value - find closing quote
        value_start++;  // Skip opening quote
        value_end = value_start;
        while (*value_end && *value_end != '"') value_end++;
        copy_len = value_end - value_start;
    } else if (*value_start == '{') {
        // Object value - find matching closing brace
        value_end = value_start + 1;
        int brace_count = 1;
        while (*value_end && brace_count > 0) {
            if (*value_end == '{')
                brace_count++;
            else if (*value_end == '}')
                brace_count--;
            value_end++;
        }
        copy_len = value_end - value_start;
    } else {
        // Numeric or other value - find next comma, brace, or end
        value_end = value_start;
        while (*value_end && *value_end != ',' && *value_end != '}' && *value_end != ']') {
            value_end++;
        }
        copy_len = value_end - value_start;
    }

    if (copy_len >= output_size) copy_len = output_size - 1;
    memcpy(output, value_start, copy_len);
    output[copy_len] = '\0';

    return true;
}

static bool extract_recipient_address(const char* recipient_object,
                                      char* address,
                                      size_t address_size) {
    // Check if it's a simple quoted address (new format): "address"
    if (recipient_object[0] == '"') {
        // Find the closing quote
        const char* address_start = recipient_object + 1;
        const char* address_end = address_start;
        while (*address_end && *address_end != '"') {
            address_end++;
        }

        if (*address_end == '"' && address_end > address_start) {
            size_t copy_len = address_end - address_start;
            if (copy_len >= address_size) copy_len = address_size - 1;

            memcpy(address, address_start, copy_len);
            address[copy_len] = '\0';
            return true;
        }
        // If we found an opening quote but no proper closing quote, this is malformed
        return false;
    }
    // Check if it's already a plain address without quotes
    else if (recipient_object[0] != '{') {
        // Plain address string - just copy it
        size_t len = 0;
        while (recipient_object[len] && recipient_object[len] != ',' &&
               recipient_object[len] != '}' && recipient_object[len] != ' ') {
            len++;
        }

        if (len > 0 && len < address_size) {
            memcpy(address, recipient_object, len);
            address[len] = '\0';
            return true;
        }
    }

    // Fallback: Look for "address: " pattern in complex object (legacy format)
    const char* address_pos = find_substring(recipient_object, "address: ");
    if (!address_pos) return false;

    const char* address_start = address_pos + strlen("address: ");
    const char* address_end = address_start;

    // Find end of address (until } or end)
    while (*address_end && *address_end != '}' && *address_end != ',') {
        address_end++;
    }

    size_t copy_len = address_end - address_start;
    if (copy_len >= address_size) copy_len = address_size - 1;

    memcpy(address, address_start, copy_len);
    address[copy_len] = '\0';

    return true;
}

static bool parse_single_operation(const char* operation_str, singlePLTOperation_t* operation) {
    if (!operation_str || !operation) return false;

    // Initialize all fields
    memset(operation, 0, sizeof(singlePLTOperation_t));
    operation->availableFields = PLT_FIELD_NONE;

    // Find the operation type (first quoted string after opening brace)
    const char* first_quote = find_substring(operation_str, "\"");
    if (!first_quote) return false;

    const char* type_start = first_quote + 1;
    const char* type_end = type_start;
    while (*type_end && *type_end != '"') type_end++;

    size_t type_len = type_end - type_start;
    if (type_len >= MAX_PLT_OPERATION_TYPE) type_len = MAX_PLT_OPERATION_TYPE - 1;
    memcpy(operation->operationType, type_start, type_len);
    operation->operationType[type_len] = '\0';

    // Parse fields based on operation type
    if (strcmp(operation->operationType, "transfer") == 0) {
        // transfer: amount, recipient (address only)
        if (extract_field_value(operation_str, "amount", operation->amount, MAX_PLT_AMOUNT_STR)) {
            operation->availableFields |= PLT_FIELD_AMOUNT;
        }

        char recipient_object[CBOR_OBJECT_BUFFER_SIZE];
        if (extract_field_value(operation_str,
                                "recipient",
                                recipient_object,
                                sizeof(recipient_object))) {
            if (extract_recipient_address(recipient_object,
                                          operation->recipient,
                                          MAX_PLT_RECIPIENT_STR)) {
                operation->availableFields |= PLT_FIELD_RECIPIENT;
            }
        }
    } else if (strcmp(operation->operationType, "addDenyList") == 0 ||
               strcmp(operation->operationType, "addAllowList") == 0 ||
               strcmp(operation->operationType, "removeAllowList") == 0 ||
               strcmp(operation->operationType, "removeDenyList") == 0) {
        // addDenyList/addAllowList/removeAllowList/removeDenyList: target (address only)
        char target_object[CBOR_OBJECT_BUFFER_SIZE];
        if (extract_field_value(operation_str, "target", target_object, sizeof(target_object))) {
            if (extract_recipient_address(target_object, operation->target, MAX_PLT_TARGET_STR)) {
                operation->availableFields |= PLT_FIELD_TARGET;
            }
        }
    } else if (strcmp(operation->operationType, "mint") == 0 ||
               strcmp(operation->operationType, "burn") == 0) {
        // mint/burn: amount only
        if (extract_field_value(operation_str, "amount", operation->amount, MAX_PLT_AMOUNT_STR)) {
            operation->availableFields |= PLT_FIELD_AMOUNT;
        } else {
            // Amount is required for mint/burn operations
            return false;
        }
    } else if (strcmp(operation->operationType, "pause") == 0 ||
               strcmp(operation->operationType, "unpause") == 0) {
        // pause/unpause: no fields
        // availableFields already set to PLT_FIELD_NONE
    } else {
        return false;
    }

    return true;
}

bool parse_plt_operation_for_ui(const char* operation_display, parsedPLTOperation_t* parsed) {
    if (!operation_display || !parsed) return false;

    // Initialize parsed structure
    memset(parsed, 0, sizeof(parsedPLTOperation_t));
    parsed->isParsed = false;

    PRINTF("Parsing PLT operation: %s\n", operation_display);

    // Count operations by counting opening braces after the initial '['
    const char* pos = operation_display;
    uint8_t operation_count = 0;
    bool more_operations_exist = false;

    // Skip initial '['
    while (*pos && *pos != '[') pos++;
    if (*pos == '[') pos++;

    // Find each operation (starts with '{')
    while (*pos && operation_count < MAX_PLT_OPERATIONS) {
        // Skip whitespace and commas
        while (*pos && (*pos == ' ' || *pos == ',' || *pos == '\n')) pos++;

        if (*pos == '{') {
            // Found start of an operation, find the end
            const char* op_start = pos;
            int brace_count = 1;
            pos++;  // Skip opening brace

            while (*pos && brace_count > 0) {
                if (*pos == '{')
                    brace_count++;
                else if (*pos == '}')
                    brace_count--;
                pos++;
            }

            if (brace_count == 0) {
                // Extract this operation string
                size_t opLen = pos - op_start;
                char op_str[CBOR_OPERATION_BUFFER_SIZE];
                if (opLen < sizeof(op_str)) {
                    memcpy(op_str, op_start, opLen);
                    op_str[opLen] = '\0';

                    // Parse this single operation
                    if (parse_single_operation(op_str, &parsed->operations[operation_count])) {
                        PRINTF("Parsed operation %d: Type=%s, Fields=0x%02X\n",
                               operation_count + 1,
                               parsed->operations[operation_count].operationType,
                               parsed->operations[operation_count].availableFields);
                        if (parsed->operations[operation_count].availableFields &
                            PLT_FIELD_AMOUNT) {
                            PRINTF("  Amount: %s\n", parsed->operations[operation_count].amount);
                        }
                        if (parsed->operations[operation_count].availableFields &
                            PLT_FIELD_RECIPIENT) {
                            PRINTF("  Recipient: %s\n",
                                   parsed->operations[operation_count].recipient);
                        }
                        if (parsed->operations[operation_count].availableFields &
                            PLT_FIELD_TARGET) {
                            PRINTF("  Target: %s\n", parsed->operations[operation_count].target);
                        }
                        operation_count++;
                    }
                }
            }
        } else if (*pos == ']') {
            // End of array
            break;
        } else {
            pos++;
        }
    }

    // Check if there are more operations beyond MAX_PLT_OPERATIONS
    if (operation_count == MAX_PLT_OPERATIONS) {
        // Continue checking for more operations without parsing them
        while (*pos && (*pos == ' ' || *pos == ',' || *pos == '\n')) pos++;
        if (*pos == '{') {
            more_operations_exist = true;
            PRINTF("Found more than %d operations, fallback to JSON display\n", MAX_PLT_OPERATIONS);
        }
    }

    parsed->operationCount = operation_count;

    if (operation_count == 0) {
        return false;
    }

    // Only mark as parsed if we can display all operations individually
    if (!more_operations_exist) {
        parsed->isParsed = true;
        PRINTF("Successfully parsed %d operations\n", operation_count);
    } else {
        parsed->isParsed = false;
        PRINTF("Too many operations (%d+), using JSON fallback display\n", operation_count);
    }

    return true;
}

/**
 * @brief Handle Protocol Level Token (PLT) signing operations
 *
 * This function processes PLT transactions which can be sent in multiple chunks due to
 * APDU size limitations. The transaction contains a token ID and CBOR-encoded operation data
 * that gets parsed and displayed to the user for approval.
 *
 * Protocol flow:
 * 1. Initial chunk (chunk=0): Contains transaction header, token ID, and CBOR length
 * 2. Subsequent chunks: Contains CBOR operation data until complete
 * 3. Final processing: Parse CBOR, generate UI display, and show to user
 *
 * @param cdata Pointer to command data buffer
 * @param lc    Length of command data (APDU data length)
 * @param chunk Chunk number (0 for initial chunk, incremented for subsequent chunks)
 * @param more  Flag indicating if more chunks are expected (true) or this is the last chunk (false)
 *
 * Initial chunk (chunk=0) data format:
 * - path_length (1 byte)
 * - derivation_path (path_length * 4 bytes)
 * - account_transaction_header (60 bytes)
 * - transaction_kind (1 byte) - must be PLT_TRANSACTION (27)
 * - token_id_length (1 byte) - length of token ID (1-255)
 * - token_id (token_id_length bytes) - token identifier
 * - cbor_length (4 bytes, big-endian) - total length of CBOR data
 * - cbor_data (remaining bytes) - start of CBOR operation data
 *
 * Subsequent chunks data format:
 * - cbor_data (lc bytes) - continuation of CBOR operation data
 *
 * Constraints:
 * - Maximum token ID length: 255 bytes (MAX_TOKEN_ID_LENGTH)
 * - Maximum CBOR data length: 900 bytes (MAX_CBOR_LENGTH)
 * - CBOR data must be valid according to CBOR specification
 * - Total display string length must not exceed 2000 bytes (MAX_PLT_DIPLAY_STR)
 *
 * Error conditions:
 * - ERROR_PLT_DATA_ERROR: Insufficient data, incomplete CBOR, or validation failures
 * - ERROR_PLT_BUFFER_ERROR: Buffer overflow, CBOR too large, or chunk size exceeded
 * - ERROR_PLT_CBOR_ERROR: CBOR parsing failures, tag parsing errors, or format issues
 *
 * @throws ERROR_PLT_DATA_ERROR If data validation fails or insufficient data provided
 * @throws ERROR_PLT_BUFFER_ERROR If buffer overflow occurs or data exceeds limits
 * @throws ERROR_PLT_CBOR_ERROR If CBOR parsing or processing fails
 *
 * @note This function updates global context (signPLTContext_t) and transaction state
 * @note User approval is required via UI display before transaction completion
 * @note All sensitive data is cleared from memory on error conditions
 */
void handle_sign_plt_transaction(uint8_t* cdata, uint8_t lc, uint8_t chunk, bool more) {
    uint8_t remaining_data_length = lc;

    if (chunk == 0) {
        explicit_bzero(ctx, sizeof(signPLTContext_t));
        ctx->currentCborLength = 0;
        ctx->totalCborLength = 0;
        // Parse and hash the header and kind
        uint8_t offset = handle_header_and_kind(cdata, remaining_data_length, PLT_TRANSACTION);
        cdata += offset;
        remaining_data_length -= offset;

        // Hash the rest of the chunk
        updateHash((cx_hash_t*)&tx_state->hash, cdata, remaining_data_length);

        // Parse token Id info
        ctx->tokenIdLength = cdata[0];
        cdata++;
        remaining_data_length--;

        if (remaining_data_length < ctx->tokenIdLength) {
            PRINTF("Not enough data left\n");
            THROW(ERROR_PLT_DATA_ERROR);
        }
        memcpy(ctx->tokenId, cdata, ctx->tokenIdLength);
        ctx->tokenId[ctx->tokenIdLength] = '\0';  // Null-terminate for display
        cdata += ctx->tokenIdLength;
        remaining_data_length -= ctx->tokenIdLength;

        // Parse OperationLength
        if (remaining_data_length < 4) {
            PRINTF("Not enough data left\n");
            THROW(ERROR_PLT_DATA_ERROR);
        }
        ctx->totalCborLength = U4BE(cdata, 0);
        cdata += 4;
        remaining_data_length -= 4;

        // Check if the OperationLength is larger than the buffer
        if (ctx->totalCborLength > sizeof(ctx->cbor)) {
            PRINTF("Cbor buffer is too small to contain the complete cbor, %d > %d\n",
                   ctx->totalCborLength,
                   sizeof(ctx->cbor));
            THROW(ERROR_PLT_BUFFER_ERROR);
        }
    }

    // Add the cbor to the context
    if (remaining_data_length > sizeof(ctx->cbor) - ctx->currentCborLength) {
        PRINTF("Cbor received is larger than the buffer, %d > %d\n",
               remaining_data_length,
               sizeof(ctx->cbor) - ctx->currentCborLength);
        THROW(ERROR_PLT_BUFFER_ERROR);
    }
    memcpy(ctx->cbor + ctx->currentCborLength, cdata, remaining_data_length);
    ctx->currentCborLength += remaining_data_length;

    if (more) {
        io_send_sw(SUCCESS);
        return;
    } else {
        if (ctx->currentCborLength == ctx->totalCborLength) {
            // Parse the cbor
            if (!parse_plt_cbor(ctx->cbor, ctx->totalCborLength)) {
                PRINTF("Cbor parsing failed\n");
                THROW(ERROR_PLT_CBOR_ERROR);
            }

            // Parse the operation for improved UI display
            parse_plt_operation_for_ui(ctx->pltOperationDisplay, &ctx->parsedOperation);

            uiPltOperationDisplay();
        } else {
            PRINTF("Cbor received is not complete, %d < %d\n",
                   ctx->currentCborLength,
                   ctx->totalCborLength);
            THROW(ERROR_PLT_DATA_ERROR);
        }
    }
}

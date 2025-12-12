// This fuzzes the handle_sign_plt_transaction function with minimal dependencies

// ========== STEP 1: STANDARD INCLUDES ==========
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// ========== STEP 2: COPY NEEDED TYPES FROM VARIOUS HEADERS ==========

// APDU response codes (from globals.h)
#define ERROR_INVALID_PARAM       0x6B03
#define ERROR_INVALID_STATE       0x6B01
#define ERROR_BUFFER_OVERFLOW     0x6B06
#define ERROR_INVALID_TRANSACTION 0x6B04
#define SUCCESS                   0x9000
#define PLT_TRANSACTION           27

// Constants from signPLT.h
#define MAX_TAG_CONTENT_SIZE        256
#define MAX_TAGS                    24
#define MAX_TAG_PARSED_CONTENT_SIZE 300
#define MAX_TOKEN_ID_LENGTH         255
#define MAX_CBOR_LENGTH             900
#define MAX_PLT_DIPLAY_STR          2000

// Transaction context structure (from signPLT.h)
typedef struct {
    uint8_t transactionType;
    uint8_t tokenId[MAX_TOKEN_ID_LENGTH];
    uint8_t tokenIdLength;
    uint8_t cbor[MAX_CBOR_LENGTH];
    size_t totalCborLength;
    size_t currentCborLength;
    char pltOperationDisplay[MAX_PLT_DIPLAY_STR];
} signPLTContext_t;

// Transaction state structure (simplified from globals.h)
typedef struct {
    uint8_t hash_state[64];  // Mock hash context
    uint8_t transactionHash[32];
    int currentInstruction;
} tx_state_t;

// Buffer structure (from various ledger headers)
typedef struct {
    const uint8_t *ptr;
    size_t size;
    size_t offset;
} buffer_t;

// Tag parsing structures (from cborStrParsing.h)
typedef struct {
    uint64_t tag_number;
    char content[MAX_TAG_CONTENT_SIZE];
    size_t content_length;
    bool is_valid;
    char parsedContent[MAX_TAG_PARSED_CONTENT_SIZE];
} tag_info_t;

typedef struct {
    tag_info_t tags[MAX_TAGS];
    size_t count;
} tag_list_t;

// ========== STEP 3: IMPORT TINYCBOR TYPES AND FUNCTIONS ==========
// We'll include the actual tinycbor library for CBOR parsing functionality

// Minimal CBOR types that we need (from tinycbor)
typedef enum {
    CborIntegerType = 0x00,
    CborByteStringType = 0x40,
    CborTextStringType = 0x60,
    CborArrayType = 0x80,
    CborMapType = 0xa0,
    CborTagType = 0xc0,
    CborSimpleType = 0xe0,
    CborBooleanType = 0xf5,
    CborNullType = 0xf6,
    CborUndefinedType = 0xf7,
    CborHalfFloatType = 0xf9,
    CborFloatType = 0xfa,
    CborDoubleType = 0xfb,
    CborInvalidType = 0xff
} CborType;

typedef uint64_t CborTag;
typedef enum {
    CborNoError = 0,
    CborUnknownError,
    CborErrorUnknownLength,
    CborErrorAdvancePastEOF,
    CborErrorIO
} CborError;

typedef struct CborValue {
    const uint8_t *ptr;
    const uint8_t *end;
    size_t remaining;
    uint16_t flags;
} CborValue;

typedef struct CborParser {
    const uint8_t *end;
    uint32_t flags;
} CborParser;

// ========== STEP 4: MOCK IMPLEMENTATIONS ==========

// Global mock contexts
static signPLTContext_t mock_plt_context;
static tx_state_t mock_tx_state;

// Mock PRINTF - just use regular printf for debugging
#define PRINTF printf

// Mock THROW - instead of crashing, just return early with appropriate value
#define THROW(exception)                                          \
    do {                                                          \
        printf("MOCK THROW: 0x%x (%s)\n", exception, #exception); \
        return 0;                                                 \
    } while (0)

#define THROW_BOOL(exception)                                     \
    do {                                                          \
        printf("MOCK THROW: 0x%x (%s)\n", exception, #exception); \
        return false;                                             \
    } while (0)

#define THROW_CBOR_ERROR(exception)                               \
    do {                                                          \
        printf("MOCK THROW: 0x%x (%s)\n", exception, #exception); \
        return CborUnknownError;                                  \
    } while (0)

#define THROW_VOID(exception)                                     \
    do {                                                          \
        printf("MOCK THROW: 0x%x (%s)\n", exception, #exception); \
        return;                                                   \
    } while (0)

// Mock LEDGER_ASSERT - print and return on failure with appropriate value
#define LEDGER_ASSERT_BOOL(condition, msg)           \
    do {                                             \
        if (!(condition)) {                          \
            printf("MOCK ASSERT FAILED: %s\n", msg); \
            return false;                            \
        }                                            \
    } while (0)

#define LEDGER_ASSERT_CBOR_ERROR(condition, msg)     \
    do {                                             \
        if (!(condition)) {                          \
            printf("MOCK ASSERT FAILED: %s\n", msg); \
            return CborUnknownError;                 \
        }                                            \
    } while (0)

#define LEDGER_ASSERT_VOID(condition, msg)           \
    do {                                             \
        if (!(condition)) {                          \
            printf("MOCK ASSERT FAILED: %s\n", msg); \
            return;                                  \
        }                                            \
    } while (0)

// Mock explicit_bzero - secure memory clearing
void explicit_bzero(void *ptr, size_t size) {
    volatile uint8_t *p = (volatile uint8_t *)ptr;
    for (size_t i = 0; i < size; i++) {
        p[i] = 0;
    }
}

// Utility macro for reading big-endian 32-bit integers
#define U4BE(buf, off)                                                                \
    ((uint32_t)(((buf)[off] << 24) | ((buf)[off + 1] << 16) | ((buf)[off + 2] << 8) | \
                ((buf)[off + 3])))

// Mock hash function - just track calls
void updateHash(void *hashContext, const unsigned char *in, unsigned int len) {
    printf("MOCK updateHash: hashing %u bytes\n", len);
    // In real implementation this would update a cryptographic hash
    // For fuzzing we just track that it was called
}

// Mock header parsing function
int handle_header_and_kind(uint8_t *cdata, uint8_t dataLength, uint8_t kind) {
    printf("MOCK handle_header_and_kind: kind=%u, dataLength=%u\n", kind, dataLength);

    // Minimal validation
    if (dataLength < 8) {
        THROW(ERROR_INVALID_TRANSACTION);
    }

    // Mock parsing - just consume some bytes for header simulation
    // Real implementation would parse derivation path and validate header
    return 8;  // Return number of bytes consumed
}

// Mock IO function
void io_send_sw(uint16_t sw) {
    printf("MOCK io_send_sw: status=0x%04x\n", sw);
}

// Mock display function - just print what would be shown
void uiPltOperationDisplay(void) {
    printf("MOCK uiPltOperationDisplay: would display: %.100s...\n",
           mock_plt_context.pltOperationDisplay);
}

// ========== STEP 5: FORMAT FUNCTIONS ==========
// Simplified versions of the format functions used in signPLT.c

int format_i64(char *dst, size_t dst_size, int64_t value) {
    int ret = snprintf(dst, dst_size, "%lld", (long long)value);
    return (ret > 0 && ret < (int)dst_size) ? ret : -1;
}

int format_u64(char *dst, size_t dst_size, uint64_t value) {
    int ret = snprintf(dst, dst_size, "%llu", (unsigned long long)value);
    return (ret > 0 && ret < (int)dst_size) ? ret : -1;
}

int format_hex(const uint8_t *src, size_t src_len, char *dst, size_t dst_size) {
    if (dst_size < src_len * 2 + 1) return -1;

    for (size_t i = 0; i < src_len; i++) {
        snprintf(dst + i * 2, 3, "%02x", src[i]);
    }
    dst[src_len * 2] = '\0';
    return 0;
}

// ========== STEP 6: SIMPLIFIED CBOR FUNCTIONS ==========
// Mock implementations of the CBOR functions used by the target

CborError cbor_parser_init(const uint8_t *buffer,
                           size_t size,
                           int flags,
                           CborParser *parser,
                           CborValue *it) {
    if (!buffer || !parser || !it) return CborUnknownError;

    parser->end = buffer + size;
    parser->flags = flags;

    it->ptr = buffer;
    it->end = buffer + size;
    it->remaining = size;
    it->flags = 0;

    return CborNoError;
}

bool cbor_value_at_end(const CborValue *it) {
    return it->ptr >= it->end;
}

CborType cbor_value_get_type(const CborValue *it) {
    if (cbor_value_at_end(it)) return CborInvalidType;

    uint8_t byte = *it->ptr;
    return (CborType)(byte & 0xe0);
}

bool cbor_value_is_container(const CborValue *it) {
    CborType type = cbor_value_get_type(it);
    return type == CborArrayType || type == CborMapType;
}

bool cbor_value_is_text_string(const CborValue *it) {
    return cbor_value_get_type(it) == CborTextStringType;
}

bool cbor_value_is_byte_string(const CborValue *it) {
    return cbor_value_get_type(it) == CborByteStringType;
}

bool cbor_value_is_negative_integer(const CborValue *it) {
    if (cbor_value_get_type(it) != CborIntegerType) return false;
    return (*it->ptr & 0x20) != 0;
}

CborError cbor_value_get_raw_integer(const CborValue *it, uint64_t *value) {
    if (!value || cbor_value_get_type(it) != CborIntegerType) return CborUnknownError;

    // Simplified - just return a mock value
    *value = 42;
    return CborNoError;
}

CborError cbor_value_get_tag(CborValue *it, CborTag *tag) {
    if (!tag || cbor_value_get_type(it) != CborTagType) return CborUnknownError;

    *tag = 121;  // Mock tag value
    return CborNoError;
}

CborError cbor_value_get_simple_type(CborValue *it, uint8_t *type) {
    if (!type) return CborUnknownError;
    *type = 0;
    return CborNoError;
}

CborError cbor_value_get_boolean(CborValue *it, bool *value) {
    if (!value) return CborUnknownError;
    *value = true;
    return CborNoError;
}

CborError cbor_value_get_double(CborValue *it, double *value) {
    if (!value) return CborUnknownError;
    *value = 3.14;
    return CborNoError;
}

CborError cbor_value_get_float(CborValue *it, float *value) {
    if (!value) return CborUnknownError;
    *value = 3.14f;
    return CborNoError;
}

CborError cbor_value_get_half_float(CborValue *it, uint16_t *value) {
    if (!value) return CborUnknownError;
    *value = 0x1234;
    return CborNoError;
}

CborError cbor_value_enter_container(CborValue *it, CborValue *recursed) {
    if (!recursed) return CborUnknownError;
    *recursed = *it;
    recursed->ptr++;
    return CborNoError;
}

CborError cbor_value_leave_container(CborValue *it, CborValue *recursed) {
    if (!recursed) return CborUnknownError;
    it->ptr = recursed->ptr;
    return CborNoError;
}

CborError cbor_value_advance_fixed(CborValue *it) {
    if (cbor_value_at_end(it)) return CborErrorAdvancePastEOF;
    it->ptr++;
    return CborNoError;
}

CborError _cbor_value_get_string_chunk(const CborValue *it,
                                       const void **chunk_ptr,
                                       size_t *chunk_len,
                                       CborValue *next) {
    if (!chunk_ptr || !chunk_len) return CborUnknownError;

    // Mock string data
    static const char mock_string[] = "mock_string";
    *chunk_ptr = mock_string;
    *chunk_len = sizeof(mock_string) - 1;

    return CborNoError;
}

// ========== STEP 7: MOCK TAG PARSING FUNCTION ==========

bool parse_tags_in_buffer(buffer_t *buffer, tag_list_t *tag_list) {
    if (!buffer || !tag_list) return false;

    // Mock implementation - just create one tag
    tag_list->count = 1;
    tag_list->tags[0].tag_number = 121;
    tag_list->tags[0].is_valid = true;
    tag_list->tags[0].content_length = 10;
    strcpy(tag_list->tags[0].content, "[1,2,3,4,5]");
    strcpy(tag_list->tags[0].parsedContent, "parsed_content");

    return true;
}

// ========== STEP 8: IMPORT ACTUAL TARGET FUNCTIONS ==========
// Copy the actual functions we want to fuzz from signPLT.c

static signPLTContext_t *ctx = &mock_plt_context;
static tx_state_t *tx_state = &mock_tx_state;

#define P1_INITIAL 0x01

static void indent(int nestingLevel) {
    while (nestingLevel--) PRINTF("  ");
}

bool cbor_read_string_or_byte_string(CborValue *it,
                                     char *output_ptr,
                                     size_t *output_size,
                                     size_t buffer_size,
                                     bool isString) {
    if (isString) {
        LEDGER_ASSERT_BOOL(cbor_value_is_text_string(it), "expected string did not get it");
    } else {
        LEDGER_ASSERT_BOOL(cbor_value_is_byte_string(it), "expected byte string did not get it");
    }

    const char *string_ptr;
    CborError err = _cbor_value_get_string_chunk(it, (const void **)&string_ptr, output_size, NULL);
    if (err) {
        return true;
    }
    PRINTF("[standalone_plt_fuzzer.c] (cbor_read_string_or_byte_string) - string_ptr: %s\n",
           string_ptr);
    PRINTF("[standalone_plt_fuzzer.c] (cbor_read_string_or_byte_string) - size: %d\n",
           (int)*output_size);

    // Check for buffer overflow
    if (*output_size > buffer_size) {
        PRINTF("Buffer overflow: string size %zu exceeds buffer size %zu\n", *output_size, buffer_size);
        return true;
    }

    // Copy the string data to the output buffer
    if (*output_size > 0 && output_ptr != NULL) {
        memcpy(output_ptr, string_ptr, *output_size);
    }

    PRINTF("[standalone_plt_fuzzer.c] (cbor_read_string_or_byte_string) - output_ptr: 0x");
    for (size_t i = 0; i < *output_size; i++) {
        PRINTF("%02x", (uint8_t)output_ptr[i]);
    }
    PRINTF("\n");
    PRINTF(
        "[standalone_plt_fuzzer.c] (cbor_read_string_or_byte_string) - output_ptr.str: "
        "%s\n",
        output_ptr);

    return false;
}
// Since in the context of the fuzzer we can't really use THROW to exit the program, we will edit
// this function's type from void to bool and return false if the buffer is too small.
bool add_char_array_to_buffer(buffer_t *dst, char *src, size_t src_size) {
    PRINTF("\n[standalone_plt_fuzzer.c] (add_char_array_to_buffer) - trying to add: %s\n", src);
    if (dst->size - dst->offset < src_size) {
        PRINTF(
            "[standalone_plt_fuzzer.c] (add_char_array_to_buffer) - src_size: 0x%08X, "
            "dst->size-offset: "
            "0x%08X\n",
            (uint32_t)src_size,
            (uint32_t)(dst->size - dst->offset));
        PRINTF("The destination buffer is too small\n");
        THROW_BOOL(ERROR_BUFFER_OVERFLOW);
    }
    memcpy((void *)(dst->ptr + dst->offset), src, src_size);
    dst->offset += src_size;
    return true;
}

// Because of the changes we made to add_char_array_to_buffer, we need to edit this function
// Now there is an if statement that checks the return value of add_char_array_to_buffer and
// returns the appropriate error code.
CborError decode_cbor_recursive(CborValue *it, int nestingLevel, buffer_t *out_buf, size_t buffer_size) {
    const char *temp;
    while (!cbor_value_at_end(it)) {
        CborError err;
        CborType type = cbor_value_get_type(it);

        // Check if we have enough space in the buffer before proceeding
        if (out_buf->offset >= buffer_size) {
            PRINTF("Buffer overflow: offset %zu >= buffer size %zu\n", out_buf->offset, buffer_size);
            return CborUnknownError;  // Return error to stop recursion
        }

        indent(nestingLevel);
        switch (type) {
            case CborArrayType:
            case CborMapType: {
                // recursive type
                CborValue recursed;
                LEDGER_ASSERT_CBOR_ERROR(cbor_value_is_container(it),
                                         "Should be a container but isn't");
                if (type == CborArrayType) {
                    temp = "[";
                } else {
                    temp = "{";
                }
                PRINTF("%s", temp);
                if (!add_char_array_to_buffer(out_buf, (char *)temp, strlen(temp))) {
                    return CborErrorIO;
                }

                err = cbor_value_enter_container(it, &recursed);
                if (err) return err;  // parse error
                err = decode_cbor_recursive(&recursed, nestingLevel + 1, out_buf, buffer_size);
                if (err) return err;  // parse error
                err = cbor_value_leave_container(it, &recursed);
                if (err) return err;  // parse error
                indent(nestingLevel);
                if (type == CborArrayType) {
                    temp = "],";
                } else {
                    temp = "},";
                }
                PRINTF("%s", temp);
                if (!add_char_array_to_buffer(out_buf, (char *)temp, strlen(temp))) {
                    return CborErrorIO;
                }
                continue;
            }
            case CborIntegerType: {
                char temp2[25];  // 25 to handle max uint64
                char temp3[30];  // 30 to handle the "Int:" prefix and max uint64
                uint64_t raw_val = 0;

                // Get the raw integer value first
                if (cbor_value_get_raw_integer(it, &raw_val) != CborNoError) {
                    PRINTF("cbor_value_get_raw_integer error\n");
                }

                if (cbor_value_is_negative_integer(it)) {
                    // Handle negative integers
                    int64_t signed_val = -(int64_t)(raw_val + 1);
                    format_i64(temp2, sizeof(temp2), signed_val);
                } else {
                    // Handle positive integers
                    format_u64(temp2, sizeof(temp2), raw_val);
                }

                snprintf(temp3, sizeof(temp3), "Int:%s,", temp2);
                if (!add_char_array_to_buffer(out_buf, temp3, strlen(temp3))) {
                    return CborErrorIO;
                }
                break;
            }

            case CborByteStringType: {
                uint8_t buf[250];
                size_t buf_len;
                err = cbor_read_string_or_byte_string(it, (char *)buf, &buf_len, sizeof(buf), false);
                if (err) return err;
                char string_value[100] = {0};
                if (format_hex(buf, buf_len, string_value, sizeof(string_value)) == -1) {
                    PRINTF("format_hex error\n");
                    THROW_CBOR_ERROR(0x0010);
                }
                if (!add_char_array_to_buffer(out_buf, (char *)"0x", 2)) {
                    return CborErrorIO;
                }
                if (!add_char_array_to_buffer(out_buf, string_value, strlen(string_value))) {
                    return CborErrorIO;
                }
                if (!add_char_array_to_buffer(out_buf, (char *)",", 1)) {
                    return CborErrorIO;
                }
                PRINTF("ByteString(%d): 0x%s\n", (int)buf_len, string_value);
                break;
            }

            case CborTextStringType: {
                uint8_t buf[250];
                size_t buf_len;
                err = cbor_read_string_or_byte_string(it, (char *)buf, &buf_len, sizeof(buf), true);
                if (err) return err;
                // null terminate the string
                buf[buf_len] = '\0';
                char temp2[256];
                snprintf(temp2, sizeof(temp2), "\"%s\",", buf);
                PRINTF("%s", temp2);
                if (!add_char_array_to_buffer(out_buf, temp2, strlen(temp2))) {
                    return CborErrorIO;
                }
                break;
            }

            case CborTagType: {
                CborTag tag;
                char temp2[16];
                cbor_value_get_tag(it, &tag);  // can't fail
                format_u64(temp2, sizeof(temp2), tag);
                char tag_str[32];
                snprintf(tag_str, sizeof(tag_str), "Tag(%s):", temp2);
                PRINTF("%s", tag_str);
                if (!add_char_array_to_buffer(out_buf, tag_str, strlen(tag_str))) {
                    return CborErrorIO;
                }

                break;
            }

            case CborSimpleType: {
                uint8_t temp_type;
                cbor_value_get_simple_type(it, &temp_type);  // can't fail
                PRINTF("simple(%d)\n", temp_type);
                break;
            }

            case CborNullType:
                temp = "null,";
                PRINTF("null");
                if (!add_char_array_to_buffer(out_buf, (char *)temp, strlen(temp))) {
                    return CborErrorIO;
                }
                break;

            case CborUndefinedType:
                temp = "undefined,";
                PRINTF("undefined");
                if (!add_char_array_to_buffer(out_buf, (char *)temp, strlen(temp))) {
                    return CborErrorIO;
                }
                break;

            case CborBooleanType: {
                bool val;
                cbor_value_get_boolean(it, &val);  // can't fail
                temp = val ? "true," : "false,";
                PRINTF("%s", temp);
                if (!add_char_array_to_buffer(out_buf, (char *)temp, strlen(temp))) {
                    return CborErrorIO;
                }
                break;
            }

            case CborDoubleType: {
                double val;
                char temp2[32];
                if (false) {
                    float f;
                    case CborFloatType:
                        cbor_value_get_float(it, &f);
                        val = f;
                } else {
                    cbor_value_get_double(it, &val);
                }
                snprintf(temp2, sizeof(temp2), "Double:0x%08x,", (uint32_t)val);
                PRINTF("Double: 0x%08x\n", (uint32_t)val);
                if (!add_char_array_to_buffer(out_buf, temp2, strlen(temp2))) {
                    return CborErrorIO;
                }
                break;
            }
            case CborHalfFloatType: {
                uint16_t val;
                char temp2[16];
                cbor_value_get_half_float(it, &val);
                snprintf(temp2, sizeof(temp2), "__f16(%04x),", val);
                PRINTF("__f16(%04x)\n", val);
                if (!add_char_array_to_buffer(out_buf, temp2, strlen(temp2))) {
                    return CborErrorIO;
                }
                break;
            }

            case CborInvalidType:
                LEDGER_ASSERT_CBOR_ERROR(false, "Can't happen");  // can't happen
                break;
        }

        err = cbor_value_advance_fixed(it);
        if (err) return err;
    }
    return CborNoError;
}

bool parse_plt_cbor(uint8_t *cbor, size_t cborLength) {
    PRINTF("[standalone_plt_fuzzer.c] (parse_plt_cbor) - cbor: ");
    for (size_t i = 0; i < cborLength; i++) {
        PRINTF("%02x", cbor[i]);
    }
    PRINTF("\n");
    PRINTF("[standalone_plt_fuzzer.c] Starting CBOR parsing, %d bytes\n", (int)cborLength);

    CborParser parser;
    CborValue it;
    CborError err;

    // Initialize parser
    err = cbor_parser_init(cbor, cborLength, 0, &parser, &it);
    if (err) {
        PRINTF("[standalone_plt_fuzzer.c] CBOR parser init failed\n");
        return false;
    }

    char temp[MAX_PLT_DIPLAY_STR] = {0};
    buffer_t out_buf = {.ptr = (const uint8_t *)temp, .size = MAX_PLT_DIPLAY_STR, .offset = 0};
    tag_list_t tag_list;  // initiate an empty tag_list_t
    err = decode_cbor_recursive(&it, 0, &out_buf, MAX_PLT_DIPLAY_STR);
    if (err) {
        PRINTF("Error while decoding cbor\n");
        THROW_BOOL(ERROR_INVALID_PARAM);
    }

    if (!parse_tags_in_buffer(&out_buf, &tag_list)) {
        PRINTF("Error while parsing cbor tags\n");
        THROW(ERROR_INVALID_PARAM);
    }
    if (sizeof(ctx->pltOperationDisplay) < out_buf.size + 1) {
        PRINTF("display str is too small for value %zu < %zu\n",
               sizeof(ctx->pltOperationDisplay),
               out_buf.size);
        THROW(ERROR_BUFFER_OVERFLOW);
    }
    memcpy(ctx->pltOperationDisplay, out_buf.ptr, out_buf.size);
    ctx->pltOperationDisplay[out_buf.size] = '\0';

    return true;
}

// ========== STEP 9: THE TARGET FUNCTION ==========
// Copy the actual handle_sign_plt_transaction function

void handle_sign_plt_transaction(uint8_t *cdata, uint8_t lc, uint8_t chunk, bool more) {
    uint8_t remainingDataLength = lc;

    PRINTF(
        "[standalone_plt_fuzzer.c] (handle_sign_plt_transaction) - Starting handling of plt "
        "transaction\n");

    if (chunk == 0) {
        explicit_bzero(ctx, sizeof(signPLTContext_t));
        ctx->currentCborLength = 0;
        ctx->totalCborLength = 0;
        PRINTF(
            "[standalone_plt_fuzzer.c] (handle_sign_plt_transaction) Initial chunk about to "
            "process\n");
        // Parse and hash the header and kind
        uint8_t offset = handle_header_and_kind(cdata, remainingDataLength, PLT_TRANSACTION);
        cdata += offset;
        remainingDataLength -= offset;

        // Hash the rest of the chunk
        updateHash((void *)&tx_state->hash_state, cdata, remainingDataLength);

        // Parse token Id info
        ctx->tokenIdLength = cdata[0];
        cdata++;
        remainingDataLength--;

        if (remainingDataLength < ctx->tokenIdLength) {
            PRINTF("Not enough data left\n");
            THROW_VOID(ERROR_INVALID_PARAM);
        }
        memcpy(ctx->tokenId, cdata, ctx->tokenIdLength);
        cdata += ctx->tokenIdLength;
        remainingDataLength -= ctx->tokenIdLength;

        PRINTF("[standalone_plt_fuzzer.c] (handle_sign_plt_transaction) - TokenID ");
        for (int i = 0; i < ctx->tokenIdLength; i++) {
            PRINTF("%02x", ctx->tokenId[i]);
        }
        PRINTF("\n");
        if (remainingDataLength < 4) {
            PRINTF("Not enough data left\n");
            THROW_VOID(ERROR_INVALID_PARAM);
        }
        // Parse OperationLength
        ctx->totalCborLength = U4BE(cdata, 0);
        PRINTF("[standalone_plt_fuzzer.c] (handle_sign_plt_transaction) - cborLength %d\n",
               (int)ctx->totalCborLength);
        cdata += 4;
        remainingDataLength -= 4;

        // Check if the OperationLength is larger than the buffer
        if (ctx->totalCborLength > sizeof(ctx->cbor)) {
            PRINTF("Cbor buffer is too small to contain the complete cbor, %d > %d\n",
                   (int)ctx->totalCborLength,
                   (int)sizeof(ctx->cbor));
            THROW_VOID(ERROR_BUFFER_OVERFLOW);
        }
    }

    // Add the cbor to the context
    if (remainingDataLength > sizeof(ctx->cbor) - ctx->currentCborLength) {
        PRINTF("Cbor received is larger than the buffer, %d > %d\n",
               remainingDataLength,
               (int)(sizeof(ctx->cbor) - ctx->currentCborLength));
        THROW_VOID(ERROR_BUFFER_OVERFLOW);
    }
    memcpy(ctx->cbor + ctx->currentCborLength, cdata, remainingDataLength);
    ctx->currentCborLength += remainingDataLength;

    if (more) {
        io_send_sw(SUCCESS);
        return;
    } else {
        if (ctx->currentCborLength == ctx->totalCborLength) {
            // Parse the cbor
            if (!parse_plt_cbor(ctx->cbor, ctx->totalCborLength)) {
                PRINTF("Cbor parsing failed\n");
                THROW_VOID(ERROR_INVALID_PARAM);
            }
            uiPltOperationDisplay();
        } else {
            PRINTF("Cbor received is not complete, %d < %d\n",
                   (int)ctx->currentCborLength,
                   (int)ctx->totalCborLength);
            THROW_VOID(ERROR_INVALID_STATE);
        }
    }
}

// ========== STEP 10: THE FUZZER ==========

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    // Need at least 16 bytes for meaningful test (header + token + cbor length)
    if (size < 16) return 0;

    printf("\n=== PLT FUZZER ITERATION (size=%zu) ===\n", size);

    // Extract fuzzer parameters from input
    uint8_t chunk = data[0] % 2;     // 0 or 1
    bool more = (data[1] % 2) == 1;  // true or false

    // Use remaining data as command data
    const uint8_t *command_data = data + 2;
    uint8_t lc = size - 2;

    printf("Fuzzing with chunk=%d, more=%s, lc=%d\n", chunk, more ? "true" : "false", lc);

    // Call the target function
    handle_sign_plt_transaction((uint8_t *)command_data, lc, chunk, more);

    printf("PLT Fuzzer completed: No crashes occurred\n");
    return 0;
}

int LLVMFuzzerInitialize(int *argc, char ***argv) {
    printf("=== CONCORDIUM PLT TRANSACTION FUZZER ===\n");
    printf("Standalone fuzzer - minimal dependencies!\n");
    printf("Target: handle_sign_plt_transaction\n\n");

    // Initialize mock contexts
    explicit_bzero(&mock_plt_context, sizeof(mock_plt_context));
    explicit_bzero(&mock_tx_state, sizeof(mock_tx_state));

    return 0;
}

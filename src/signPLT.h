#pragma once

#include <stdint.h>
#include <stdbool.h>

#define MAX_TAG_CONTENT_SIZE 256
#define MAX_TAGS             24

#define MAX_TAG_PARSED_CONTENT_SIZE 300
#define MAX_TOKEN_ID_LENGTH         255
#define MAX_CBOR_LENGTH             900
#define MAX_PLT_DIPLAY_STR          2000

// Buffer size constants for CBOR parsing and display
#define CBOR_INTEGER_BUFFER_SIZE   25   // Buffer size for uint64 integer formatting
#define CBOR_INTEGER_PREFIX_SIZE   30   // Buffer size for "Int:" prefix + uint64 integer
#define CBOR_STRING_BUFFER_SIZE    250  // Buffer size for CBOR string/byte string parsing
#define CBOR_HEX_DISPLAY_SIZE      100  // Buffer size for hex string display
#define CBOR_TEXT_DISPLAY_SIZE     256  // Buffer size for text string display
#define CBOR_TAG_STRING_SIZE       32   // Buffer size for tag string formatting
#define CBOR_PATTERN_BUFFER_SIZE   64   // Buffer size for pattern matching
#define CBOR_OBJECT_BUFFER_SIZE    256  // Buffer size for object display
#define CBOR_OPERATION_BUFFER_SIZE 512  // Buffer size for operation display
#define CBOR_FLOAT_DISPLAY_SIZE    32   // Buffer size for float/double display
#define MAX_PLT_OPERATION_TYPE     32
#define MAX_PLT_AMOUNT_STR         32
#define MAX_PLT_RECIPIENT_STR      128
#define MAX_PLT_TARGET_STR         128
#define MAX_PLT_OPERATIONS         10
#define MAX_PLT_SUMMARY_STR        64

void handle_sign_plt_transaction(uint8_t* cdata, uint8_t lc, uint8_t chunk, bool more);

typedef enum {
    PLT_FIELD_NONE = 0,
    PLT_FIELD_AMOUNT = 1,
    PLT_FIELD_RECIPIENT = 2,
    PLT_FIELD_TARGET = 4
} pltFieldFlags_t;

typedef struct {
    char operationType[MAX_PLT_OPERATION_TYPE];
    char amount[MAX_PLT_AMOUNT_STR];
    char recipient[MAX_PLT_RECIPIENT_STR];
    char target[MAX_PLT_TARGET_STR];
    pltFieldFlags_t availableFields;  // Bit flags indicating which fields are available
} singlePLTOperation_t;

typedef struct {
    singlePLTOperation_t operations[MAX_PLT_OPERATIONS];
    uint8_t operationCount;
    bool isParsed;
} parsedPLTOperation_t;

typedef struct {
    uint8_t transactionType;
    uint8_t tokenId[MAX_TOKEN_ID_LENGTH];
    uint8_t tokenIdLength;
    uint8_t cbor[MAX_CBOR_LENGTH];
    size_t totalCborLength;
    size_t currentCborLength;
    char pltOperationDisplay[MAX_PLT_DIPLAY_STR];
    parsedPLTOperation_t parsedOperation;
} signPLTContext_t;

bool parse_plt_operation_for_ui(const char* operationDisplay, parsedPLTOperation_t* parsed);

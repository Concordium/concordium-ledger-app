#pragma once

#include <stddef.h>  // size_t
#include <stdint.h>  // uint*_t

#define MAX_TX_LEN   510
#define ADDRESS_LEN  32
#define MAX_MEMO_LEN 465  // 510 - ADDRESS_LEN - 2*SIZE(U64) - SIZE(MAX_VARINT)

typedef enum {
    PARSING_OK = 1,
    MEMO_PARSING_ERROR = -1,
    WRONG_LENGTH_ERROR = -7,
    TYPE_PARSING_ERROR = -8,
    SENDER_PARSING_ERROR = -9,
    RECIPIENT_PARSING_ERROR = -10,
    AMOUNT_PARSING_ERROR = -11,
    PARSING_ERROR = -12
} parser_status_e;

typedef struct {
    uint64_t value;     /// amount value (8 bytes)
    uint8_t *recipient; /// pointer to recipient (32 bytes)
    uint8_t *sender;    /// pointer to sender (32 bytes)
} simple_transfer_t;

typedef struct {
    uint64_t value;     /// amount value (8 bytes)
    uint8_t *recipient; /// pointer to recipient (32 bytes)
    uint8_t *sender;    /// pointer to sender (32 bytes)
    uint8_t *memo;      /// pointer to memo (variable length)
    uint64_t memo_len;  /// length of memo (8 bytes)
} simple_transfer_with_memo_t;

// TODO: ADD OTHER TRANSACTION TYPES HERE
typedef enum {
    TRANSACTION_TYPE_SIMPLE_TRANSFER = 3,
} transaction_type_e;

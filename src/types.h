#pragma once

#include <stddef.h>  // size_t
#include <stdint.h>  // uint*_t

#include "bip32.h"

#include "constants.h"
#include "transaction/types.h"

/**
 * Enumeration with expected INS of APDU commands.
 */
typedef enum {
    VERIFY_ADDRESS = 0x00,       /// verify address
    GET_VERSION = 0x03,          /// version of the application
    GET_APP_NAME = 0x04,         /// name of the application
    GET_PUBLIC_KEY = 0x05,       /// public key of corresponding BIP32 path
    SIGN_SIMPLE_TRANSFER = 0x06  /// sign simple transfer with BIP32 path
} command_e;
/**
 * Enumeration with parsing state.
 */
typedef enum {
    STATE_NONE,     /// No state
    STATE_PARSED,   /// Transaction data parsed
    STATE_APPROVED  /// Transaction data approved
} state_e;

/**
 * Enumeration with user request type.
 */
typedef enum {
    CONFIRM_ADDRESS,     /// confirm address derived from public key
    CONFIRM_TRANSACTION  /// confirm transaction information
} request_type_e;

/**
 * Structure for public key context information.
 */
typedef struct {
    uint8_t raw_public_key[65];  /// format (1), x-coordinate (32), y-coodinate (32)
    uint8_t chain_code[32];      /// for public key derivation
} pubkey_ctx_t;

/**
 * Structure for transaction information context.
 */
typedef struct {
    uint8_t raw_tx[MAX_TRANSACTION_LEN];  /// raw transaction serialized
    size_t raw_tx_len;                    /// length of raw transaction
    uint8_t type;                         /// transaction type
    union {
        simple_transfer_t simple_transfer;
        simple_transfer_with_memo_t simple_transfer_with_memo;
        // Add other transaction types here as needed
    } transaction;
    uint8_t m_hash[32];                  /// message hash digest
    uint8_t signature[MAX_DER_SIG_LEN];  /// transaction signature encoded in DER
    uint8_t signature_len;               /// length of transaction signature
    // uint8_t v;                            /// parity of y-coordinate of R in ECDSA signature
} transaction_ctx_t;

/**
 * Structure for verify address context.
 */
typedef struct {
    char address[57];
    uint32_t idp_index;
    uint32_t identity_index;
    uint32_t credential_counter;
} verify_address_ctx_t;

/**
 * Structure for global context.
 */
typedef struct {
    state_e state;  /// state of the context
    union {
        pubkey_ctx_t pk_info;                      /// public key context
        transaction_ctx_t tx_info;                 /// transaction context
        verify_address_ctx_t verify_address_info;  /// verify address context
    };
    request_type_e req_type;                        /// user request
    uint32_t bip32_path[MAX_BIP32_PATH_SUPPORTED];  /// BIP32 path
    uint8_t bip32_path_len;                         /// length of BIP32 path
} global_ctx_t;

#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <os.h>
#include <cx.h>
#include <ux.h>
#include <io.h>
#include <format.h>
#include <os_io_seproxyhal.h>
#include <lcx_hash.h>
#include <parser.h>
#include <base58.h>
#include <glyphs.h>

// Common buffer size constants used across the application
#define COMMON_PRIVATE_KEY_SIZE    32   // Standard private key size
#define COMMON_PUBLIC_KEY_SIZE     32   // Standard public key size
#define COMMON_SIGNATURE_SIZE      64   // Standard signature size
#define COMMON_HASH_SIZE           32   // Standard hash size
#define COMMON_ADDRESS_SIZE        57   // Standard address size
#define COMMON_DISPLAY_SIZE        255  // Standard display buffer size
#define COMMON_AMOUNT_DISPLAY_SIZE 30   // Standard amount display size
#define COMMON_URL_DISPLAY_SIZE    256  // Standard URL display size
#define COMMON_MODULE_REF_SIZE     32   // Standard module reference size
#define COMMON_THRESHOLD_SIZE      4    // Standard threshold size
#define COMMON_TIMESTAMP_SIZE      8    // Standard timestamp size
#define COMMON_COMMISSION_SIZE     8    // Standard commission rate size
#include <limits.h>
#include <format.h>

#ifdef HAVE_NBGL
#include <nbgl_use_case.h>
#endif

#include "display.h"
#include "handler.h"
#include "menu.h"
#include "util.h"
#include "sign.h"
#include "numberHelpers.h"
#include "base58check.h"
#include "time.h"

#include "exportPrivateKey.h"
#include "verifyAddress.h"
#include "getPublicKey.h"
#include "signConfigureBaker.h"
#include "signConfigureDelegation.h"
#include "signCredentialDeployment.h"
#include "signPublicInformationForIp.h"
#include "signTransfer.h"
#include "signTransferToPublic.h"
#include "signTransferWithSchedule.h"
#include "signRegisterData.h"
#include "deployModule.h"
#include "initContract.h"
#include "updateContract.h"
#include "signPLT.h"

#define LEGACY_PURPOSE   1105
#define LEGACY_COIN_TYPE 0
#define NEW_PURPOSE      44
#define NEW_COIN_TYPE    919

#define MAX_CDATA_LENGTH 255

#define ACCOUNT_TRANSACTION_HEADER_LENGTH 60
#define UPDATE_HEADER_LENGTH              28

/**
 * Instruction class of the Concordium application.
 */
#define CLA 0xE0

/**
 * Length of APPNAME variable in the Makefile.
 */
#define APPNAME_LEN (sizeof(APPNAME) - 1)

/**
 * Maximum length of MAJOR_VERSION || MINOR_VERSION || PATCH_VERSION.
 */
#define APPVERSION_LEN 3

/**
 * Maximum length of application name.
 */
#define MAX_APPNAME_LEN 64

/**
 * Key length of (Public Key || Verification Key || Account Key)
 */
#define KEY_LENGTH 32

/**
 * P2 value for more data
 */
#define P2_MORE 0x80

typedef enum {
    LEGACY_ID_CRED_SEC = 0,
    LEGACY_PRF_KEY = 1,
    // New path
    NEW_ID_CRED_SEC = 2,
    NEW_PRF_KEY = 3,
    NEW_SIGNATURE_BLINDING_RANDOMNESS = 4,
    NEW_COMMITMENT_RANDOMNESS = 5,
} derivation_path_keys_t;

typedef enum {
    DEPLOY_MODULE = 0,
    INIT_CONTRACT = 1,
    UPDATE_CONTRACT = 2,
    TRANSFER = 3,
    UPDATE_CREDENTIAL_KEYS = 13,
    TRANSFER_TO_PUBLIC = 18,
    TRANSFER_WITH_SCHEDULE = 19,
    UPDATE_CREDENTIALS = 20,
    REGISTER_DATA = 21,
    TRANSFER_WITH_MEMO = 22,
    TRANSFER_WITH_SCHEDULE_WITH_MEMO = 24,
    CONFIGURE_BAKER = 25,
    CONFIGURE_DELEGATION = 26,
    PLT_TRANSACTION = 27,
} transactionKind_e;

typedef struct {
    uint8_t identity;
    uint8_t accountIndex;

    // Max length of path is 8. Currently we expect to receive the root, i.e. purpose and cointype
    // as well. This could be refactored into having those values hardcoded if we determine they
    // will be static.
    uint8_t pathLength;
    uint32_t keyDerivationPath[8];
    uint32_t rawKeyDerivationPath[8];
} keyDerivationPath_t;
extern keyDerivationPath_t path;

// Helper object used when computing the hash of a transaction,
// and to keep track of the state of a multi command APDU flow.
typedef struct {
    cx_sha256_t hash;
    uint8_t transactionHash[COMMON_HASH_SIZE];
    int currentInstruction;
} tx_state_t;
extern tx_state_t global_tx_state;

// Helper struct that is used to hold the account sender
// address from an account transaction header.
typedef struct {
    uint8_t sender[COMMON_ADDRESS_SIZE];
} accountSender_t;
extern accountSender_t global_account_sender;

typedef struct {
    uint32_t cborLength;
    uint32_t displayUsed;
    uint8_t display[COMMON_DISPLAY_SIZE];
    uint8_t majorType;
} cborContext_t;

typedef struct {
    union {
        signTransferContext_t signTransferContext;
        signTransferWithScheduleContext_t signTransferWithScheduleContext;
        signRegisterData_t signRegisterData;
        signPLTContext_t signPLTContext;
    };
    cborContext_t cborContext;

} transactionWithDataBlob_t;

/**
 * As the memory we have available is very limited, the context for each instruction is stored
 * in a shared global union, so that we do not use more memory than that of the most memory
 * consuming instruction context.
 */
typedef union {
    exportPrivateKeyContext_t exportPrivateKeyContext;
    exportPublicKeyContext_t exportPublicKeyContext;
    verifyAddressContext_t verifyAddressContext;

    signPublicInformationForIp_t signPublicInformationForIp;
    signCredentialDeploymentContext_t signCredentialDeploymentContext;

    signTransferToPublic_t signTransferToPublic;
    signConfigureBaker_t signConfigureBaker;
    signConfigureDelegationContext_t signConfigureDelegation;
    deployModule_t deployModule;
    initContract_t initContract;
    updateContract_t updateContract;
    transactionWithDataBlob_t withDataBlob;
} instructionContext;
extern instructionContext global;

typedef struct internal_storage_t {
    uint8_t dummy1_allowed;
    uint8_t dummy2_allowed;
    uint8_t initialized;
} internal_storage_t;

extern const internal_storage_t N_storage_real;

#define N_storage (*(volatile internal_storage_t *)PIC(&N_storage_real))

enum {
    // Successful codes
    SUCCESS = 0x9000,

    // Error codes
    ERROR_NO_APDU_RECEIVED = 0x6982,
    ERROR_REJECTED_BY_USER = 0x6985,
    ERROR_INVALID_CLA = 0x6E00,

    ERROR_INVALID_STATE = 0x6B01,
    ERROR_INVALID_PATH = 0x6B02,
    ERROR_INVALID_PARAM = 0x6B03,
    ERROR_INVALID_TRANSACTION = 0x6B04,
    ERROR_UNSUPPORTED_CBOR = 0x6B05,
    ERROR_BUFFER_OVERFLOW = 0x6B06,
    ERROR_FAILED_CX_OPERATION = 0x6B07,
    ERROR_INVALID_INSTRUCTION = 0x6D00,
    ERROR_INVALID_SOURCE_LENGTH = 0x6B08,
    ERROR_INVALID_NAME_LENGTH = 0x6B0A,
    ERROR_INVALID_PARAMS_LENGTH = 0x6B0B,
    ERROR_INVALID_MODULE_REF = 0x6B09,
    ERROR_INVALID_COININFO = 0x6B0C,

    // PLT-specific error codes
    ERROR_PLT_CBOR_ERROR = 0x6B0D,    // PLT CBOR parsing or processing error
    ERROR_PLT_BUFFER_ERROR = 0x6B0E,  // PLT buffer overflow or size error
    ERROR_PLT_DATA_ERROR = 0x6B0F,    // PLT data validation or integrity error

    // Error codes from the Ledger firmware
    ERROR_DEVICE_LOCKED = 0x530C,
    SW_WRONG_DATA_LENGTH = 0x6A87
};

#ifndef _GLOBALS_H_
#define _GLOBALS_H_

#include <stdbool.h>

#include "os.h"
#include "cx.h"
#include "exportPrivateKey.h"
#include "exportData.h"
#include "verifyAddress.h"

#include "getPublicKey.h"
#include "displayCbor.h"
#include "signConfigureBaker.h"
#include "signConfigureDelegation.h"
#include "signCredentialDeployment.h"
#include "signEncryptedAmountTransfer.h"
#include "signPublicInformationForIp.h"
#include "signTransfer.h"
#include "signTransferToEncrypted.h"
#include "signTransferToPublic.h"
#include "signTransferWithSchedule.h"
#include "signRegisterData.h"

#include "ux.h"

#define CONCORDIUM_PURPOSE_LEGACY   1105
#define CONCORDIUM_COIN_TYPE_LEGACY 0

#define CONCORDIUM_PURPOSE           44
#define CONCORDIUM_COIN_TYPE_MAINNET 919
#define CONCORDIUM_COIN_TYPE_TESTNET 1

#define MAX_CDATA_LENGTH 255

#define ACCOUNT_TRANSACTION_HEADER_LENGTH 60
#define UPDATE_HEADER_LENGTH              28

typedef enum {
    DEPLOY_MODULE = 0,
    INIT_CONTRACT = 1,
    UPDATE = 2,
    TRANSFER = 3,
    UPDATE_CREDENTIAL_KEYS = 13,
    ENCRYPTED_AMOUNT_TRANSFER = 16,
    TRANSFER_TO_ENCRYPTED = 17,
    TRANSFER_TO_PUBLIC = 18,
    TRANSFER_WITH_SCHEDULE = 19,
    UPDATE_CREDENTIALS = 20,
    REGISTER_DATA = 21,
    TRANSFER_WITH_MEMO = 22,
    ENCRYPTED_AMOUNT_TRANSFER_WITH_MEMO = 23,
    TRANSFER_WITH_SCHEDULE_WITH_MEMO = 24,
    CONFIGURE_BAKER = 25,
    CONFIGURE_DELEGATION = 26
} transactionKind_e;

typedef struct {
    uint8_t identity;
    uint8_t accountIndex;

    // Max length of path is 8. Currently we expect to receive the root, i.e. purpose and cointype as well.
    // This could be refactored into having those values hardcoded if we determine they will be static.
    uint8_t pathLength;
    uint32_t keyDerivationPath[8];
    uint32_t rawKeyDerivationPath[8];
} keyDerivationPath_t;
extern keyDerivationPath_t path;

// Helper object used when computing the hash of a transaction,
// and to keep track of the state of a multi command APDU flow.
typedef struct {
    cx_sha256_t hash;
    uint8_t transactionHash[32];
    int currentInstruction;
} tx_state_t;
extern tx_state_t global_tx_state;

// Helper struct that is used to hold the account sender
// address from an account transaction header.
typedef struct {
    uint8_t sender[57];
} accountSender_t;
extern accountSender_t global_account_sender;

typedef struct {
    union {
        signTransferContext_t signTransferContext;
        signEncryptedAmountToTransfer_t signEncryptedAmountToTransfer;
        signTransferWithScheduleContext_t signTransferWithScheduleContext;
        signRegisterData_t signRegisterData;
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
    exportDataContext_t exportDataContext;
    exportPublicKeyContext_t exportPublicKeyContext;
    verifyAddressContext_t verifyAddressContext;

    signPublicInformationForIp_t signPublicInformationForIp;
    signCredentialDeploymentContext_t signCredentialDeploymentContext;

    signTransferToEncrypted_t signTransferToEncrypted;
    signTransferToPublic_t signTransferToPublic;
    signConfigureBaker_t signConfigureBaker;
    signConfigureDelegationContext_t signConfigureDelegation;

    transactionWithDataBlob_t withDataBlob;
} instructionContext;
extern instructionContext global;

#endif


// ========== STEP 1: STANDARD INCLUDES ==========
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ========== STEP 2: COPY NEEDED TYPES FROM GLOBALS.H ==========
// Instead of including the whole globals.h, we copy just what we need

// Derivation path key types (copied from globals.h)
typedef enum {
    NEW_ID_CRED_SEC = 0,
    NEW_PRF_KEY = 1,
    NEW_SIGNATURE_BLINDING_RANDOMNESS = 2,
    NEW_COMMITMENT_RANDOMNESS = 3
} derivation_path_key_t;

// APDU response codes (copied from globals.h)
#define ERROR_INVALID_PARAM 0x6B00
#define ERROR_INVALID_PATH  0x6A80
#define SUCCESS             0x9000

// Constants from exportPrivateKey.h
#define MAX_DERIVATION_PATH_LENGTH  6
#define MAX_KEYS_TO_EXPORT          3
#define LENGTH_AND_PRIVATE_KEY_SIZE 33

// Purpose constants (from exportPrivateKey.h)
#define P1_IDENTITY_CREDENTIAL_CREATION 0x00
#define P1_ACCOUNT_CREATION             0x01
#define P1_ID_RECOVERY                  0x02
#define P1_ACCOUNT_CREDENTIAL_DISCOVERY 0x03
#define P1_CREATION_OF_ZK_PROOF         0x04

// Derivation path constants
#define NEW_PURPOSE     1105
#define NEW_COIN_TYPE   919
#define HARDENED_OFFSET 0x80000000

// ========== STEP 3: MOCK IMPLEMENTATIONS ==========

// Mock PRINTF - just use regular printf for debugging
#define PRINTF printf

// Mock THROW - instead of crashing, just return early
#define THROW(exception)                                          \
    do {                                                          \
        printf("MOCK THROW: 0x%x (%s)\n", exception, #exception); \
        return;                                                   \
    } while (0)

// Mock explicit_bzero - secure memory clearing
void explicit_bzero(void *ptr, size_t size) {
    if (ptr) {
        volatile uint8_t *p = (volatile uint8_t *)ptr;
        for (size_t i = 0; i < size; i++) {
            p[i] = 0;
        }
    }
}

// Mock bin2dec - convert binary to decimal string
size_t bin2dec(uint8_t *dst, size_t dst_size, uint32_t value) {
    int ret = snprintf((char *)dst, dst_size, "%u", value);
    return (ret > 0 && ret < (int)dst_size) ? ret + 1 : 0;
}

// Mock number helpers - simplified versions
void numberToText(uint8_t *dst, size_t dst_size, uint64_t number) {
    snprintf((char *)dst, dst_size, "%llu", number);
}

uint8_t lengthOfNumber(uint64_t number) {
    if (number == 0) return 1;
    uint8_t length = 0;
    while (number > 0) {
        length++;
        number /= 10;
    }
    return length;
}

// Mock crypto functions - return fake but valid-looking keys
int getPrivateKey(uint32_t *derivationPath,
                  uint8_t pathLength,
                  uint8_t *privateKeyArray,
                  uint8_t *privateKey) {
    printf("MOCK getPrivateKey: path_length=%d\n", pathLength);
    if (privateKey) {
        memset(privateKey, 0xAB, 32);  // Fake private key
    }
    return 0;
}

int getBlsPrivateKey(uint32_t *derivationPath,
                     uint8_t pathLength,
                     uint8_t *privateKeyArray,
                     uint8_t *privateKey) {
    printf("MOCK getBlsPrivateKey: path_length=%d\n", pathLength);
    if (privateKey) {
        memset(privateKey, 0xCD, 32);  // Fake BLS private key
    }
    return 0;
}

// Utility macro for reading big-endian 32-bit integers
#define U4BE(buf, off)                                                                \
    ((uint32_t)(((buf)[off] << 24) | ((buf)[off + 1] << 16) | ((buf)[off + 2] << 8) | \
                ((buf)[off + 3])))

// ========== STEP 4: MOCK exportNewPathPrivateKeysForPurpose ==========
// Simplified version of the real function

int exportNewPathPrivateKeysForPurpose(derivation_path_key_t purpose,
                                       uint32_t identityProvider,
                                       uint32_t identity,
                                       uint32_t account,
                                       uint8_t *outputPrivateKey,
                                       size_t outputPrivateKeySize) {
    printf("MOCK exportNewPathPrivateKeysForPurpose: purpose=%d, idp=%u, id=%u, account=%u\n",
           purpose,
           identityProvider,
           identity,
           account);

    // Build derivation path
    uint32_t derivationPath[MAX_DERIVATION_PATH_LENGTH];
    uint8_t pathLength = 4;

    derivationPath[0] = NEW_PURPOSE | HARDENED_OFFSET;
    derivationPath[1] = NEW_COIN_TYPE | HARDENED_OFFSET;
    derivationPath[2] = identityProvider | HARDENED_OFFSET;
    derivationPath[3] = identity | HARDENED_OFFSET;

    // Extend path based on purpose
    switch (purpose) {
        case NEW_ID_CRED_SEC:
        case NEW_PRF_KEY:
        case NEW_SIGNATURE_BLINDING_RANDOMNESS:
            derivationPath[pathLength++] = purpose | HARDENED_OFFSET;
            break;
        case NEW_COMMITMENT_RANDOMNESS:
            derivationPath[pathLength++] = purpose | HARDENED_OFFSET;
            derivationPath[pathLength++] = account | HARDENED_OFFSET;
            break;
        default:
            printf("Invalid purpose: %d\n", purpose);
            return 0;
    }

    // Generate fake private keys
    uint8_t fakeKey[32];
    uint8_t tx = 0;

    if (outputPrivateKeySize < LENGTH_AND_PRIVATE_KEY_SIZE) {
        return 0;
    }

    // Call our mock crypto function
    getPrivateKey(derivationPath, pathLength, NULL, fakeKey);

    // Write length + key to output
    outputPrivateKey[tx++] = 32;  // Key length
    memcpy(outputPrivateKey + tx, fakeKey, 32);
    tx += 32;

    return tx;
}

// ========== STEP 5: THE MAIN TARGET FUNCTION ==========
// Simplified version of handleExportPrivateKeyNewPath

void handleExportPrivateKeyNewPath(uint8_t *dataBuffer,
                                   uint8_t p1,
                                   uint8_t lc,
                                   volatile unsigned int *flags) {
    printf("=== handleExportPrivateKeyNewPath ===\n");
    printf("p1=%d, lc=%d\n", p1, lc);

    // Validate p1 parameter
    if (p1 != P1_IDENTITY_CREDENTIAL_CREATION && p1 != P1_ACCOUNT_CREATION &&
        p1 != P1_ID_RECOVERY && p1 != P1_ACCOUNT_CREDENTIAL_DISCOVERY &&
        p1 != P1_CREATION_OF_ZK_PROOF) {
        THROW(ERROR_INVALID_PARAM);
    }

    size_t offset = 0;
    uint8_t remainingDataLength = lc;

    // Extract identity provider (4 bytes)
    if (remainingDataLength < 4) {
        THROW(ERROR_INVALID_PATH);
    }
    uint32_t identityProvider = U4BE(dataBuffer, offset);
    offset += 4;
    remainingDataLength -= 4;
    printf("identityProvider=%u\n", identityProvider);

    // Extract identity (4 bytes)
    if (remainingDataLength < 4) {
        THROW(ERROR_INVALID_PATH);
    }
    uint32_t identity = U4BE(dataBuffer, offset);
    offset += 4;
    remainingDataLength -= 4;
    printf("identity=%u\n", identity);
    // Extract account (if needed)
    uint32_t account = 0xFFFFFFFF;
    if (p1 == P1_ACCOUNT_CREATION || p1 == P1_CREATION_OF_ZK_PROOF) {
        if (remainingDataLength < 4) {
            THROW(ERROR_INVALID_PATH);
        }
        account = U4BE(dataBuffer, offset);
        printf("account=%u\n", account);
    }

    // Generate the private keys
    uint8_t outputBuffer[MAX_KEYS_TO_EXPORT * LENGTH_AND_PRIVATE_KEY_SIZE];
    int bytesWritten = exportNewPathPrivateKeysForPurpose(
        (derivation_path_key_t)(p1 % 4),  // Convert p1 to valid purpose
        identityProvider,
        identity,
        account,
        outputBuffer,
        sizeof(outputBuffer));

    printf("Generated %d bytes of private key data\n", bytesWritten);

    // In real implementation, this would call UI and send the keys
    // For fuzzing, we just print what would have been dispatched to the UI and say success
    printf("Display would have been: identityProvider=%u, identity=%u, account=%u\n",
           identityProvider,
           identity,
           account);

    printf("=== handleExportPrivateKeyNewPath completed successfully ===\n");
}

// ========== STEP 6: THE FUZZER ==========

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    // Need at least 9 bytes for a meaningful test
    if (size < 9) return 0;

    printf("\n=== FUZZER ITERATION (size=%zu) ===\n", size);

    // Extract parameters from fuzz input
    uint8_t p1 = data[0];

    // Clamp lc to available data
    uint8_t lc = size - 1;

    const uint8_t *command_data = data + 1;

    printf("Fuzzing with p1=%d, lc=%d\n", p1, lc);

    // Call the target function
    volatile unsigned int flags = 0;
    handleExportPrivateKeyNewPath((uint8_t *)command_data, p1, lc, &flags);

    printf("Fuzzer completed: No weird stuff happened\n");
    return 0;
}

int LLVMFuzzerInitialize(int *argc, char ***argv) {
    printf("=== CONCORDIUM EXPORT PRIVATE KEY FUZZER ===\n");
    printf("Standalone fuzzer - no external dependencies!\n");
    printf("Target: handleExportPrivateKeyNewPath\n\n");
    return 0;
}

#pragma once

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
 * Maximum transaction length (bytes).
 */
#define MAX_TRANSACTION_LEN 510

/**
 * Maximum signature length (bytes).
 */
#define MAX_DER_SIG_LEN 72

/**
 * Exponent used to convert mCCD to CCD unit (N CCD = N * 10^3 mCCD).
 */
#define EXPONENT_SMALLEST_UNIT 3

/**
 * Hardened offset for BIP32 path.
 */
#define HARDENED_OFFSET 0x80000000

/**
 * BIP44 purpose value for legacy derivation paths.
 */
#define LEGACY_PURPOSE 1105

/**
 * Coin type value for Concordium in legacy derivation paths.
 */
#define LEGACY_COIN_TYPE 0

/**
 * Account subtree index for legacy derivation paths.
 */
#define LEGACY_ACCOUNT_SUBTREE 0

/**
 * Normal account index used in legacy derivation paths.
 */
#define LEGACY_NORMAL_ACCOUNT 0

/**
 * Legacy key indexes used in derivation paths.
 */
enum legacy_key_indexes {
    LEGACY_ID_CRED = 0,
    LEGACY_PRF_KEY = 1,
    LEGACY_SIGN_KEY = 2,
    LEGACY_R = 3,
    LEGACY_M0 = 4
};

/**
 * Purpose value for new address format.
 */
#define NEW_PURPOSE 44

/**
 * Coin type value for new address format.
 */
#define NEW_COIN_TYPE 919

enum new_key_indexes { NEW_SIGN_KEY = 0, NEW_ID_CRED = 1, NEW_PRF_KEY = 2, NEW_M0 = 3, NEW_R = 4 };

/**
 * Length of the credential ID.
 */
#define CREDENTIAL_ID_LEN 48

/**
 * Length of the address.
 */
#define CONCORDIUM_ADDRESS_LEN 32

/**
 * Concordium version byte.
 */
#define CONCORDIUM_VERSION_BYTE 1

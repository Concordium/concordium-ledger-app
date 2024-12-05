#ifndef _CONCORDIUM_APP_ERROR_CODES_H_
#define _CONCORDIUM_APP_ERROR_CODES_H_

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

    // Error codes from the Ledger firmware
    ERROR_DEVICE_LOCKED = 0x530C,
    SW_WRONG_DATA_LENGTH = 0x6A87
};
#endif

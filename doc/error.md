# Error Codes

This document lists all error codes that can be returned by the Concordium Ledger application.

## Success Codes

| Code   | Name      | Description |
|--------|-----------|-------------|
| `0x9000` | `SUCCESS` | Operation completed successfully |

## Error Categories

### APDU Protocol Errors

| Code   | Name                    | Description |
|--------|------------------------|-------------|
| `0x6982` | `ERROR_NO_APDU_RECEIVED` | No APDU was received |
| `0x6985` | `ERROR_REJECTED_BY_USER` | Operation was rejected by the user |
| `0x6E00` | `ERROR_INVALID_CLA`      | Invalid CLA byte in APDU header |
| `0x6D00` | `ERROR_INVALID_INSTRUCTION` | Invalid instruction byte in APDU header |
| `0x6A87` | `SW_WRONG_DATA_LENGTH`   | Incorrect data length |

### Transaction and Parameter Errors

| Code   | Name                        | Description |
|--------|----------------------------|-------------|
| `0x6B01` | `ERROR_INVALID_STATE`      | Operation attempted in invalid state |
| `0x6B02` | `ERROR_INVALID_PATH`       | Invalid derivation path |
| `0x6B03` | `ERROR_INVALID_PARAM`      | Invalid parameter provided |
| `0x6B04` | `ERROR_INVALID_TRANSACTION` | Invalid transaction format or data |
| `0x6B05` | `ERROR_UNSUPPORTED_CBOR`   | Unsupported CBOR format or operation |
| `0x6B06` | `ERROR_BUFFER_OVERFLOW`    | Buffer overflow occurred |
| `0x6B07` | `ERROR_FAILED_CX_OPERATION` | Failed cryptographic operation |
| `0x6B08` | `ERROR_INVALID_SOURCE_LENGTH` | Invalid source length |
| `0x6B09` | `ERROR_INVALID_MODULE_REF` | Invalid module reference |
| `0x6B0A` | `ERROR_INVALID_NAME_LENGTH` | Invalid name length |
| `0x6B0B` | `ERROR_INVALID_PARAMS_LENGTH` | Invalid parameters length |
| `0x6B0C` | `ERROR_INVALID_COININFO`   | Invalid coin information |

### Device State Errors

| Code   | Name                | Description |
|--------|-------------------|-------------|
| `0x530C` | `ERROR_DEVICE_LOCKED` | Device is locked |

### BIP32 Display Errors

| Code   | Name                        | Description |
|--------|----------------------------|-------------|
| `0xB001` | `SW_DISPLAY_BIP32_PATH_FAIL` | Failed to convert BIP32 path to string |
| `0xB002` | `SW_DISPLAY_ADDRESS_FAIL`    | Failed to convert address to string |
| `0xB003` | `SW_DISPLAY_AMOUNT_FAIL`     | Failed to convert amount to string |
| `0xB004` | `SW_WRONG_TX_LENGTH`         | Wrong raw transaction length |
| `0xB005` | `SW_TX_PARSING_FAIL`         | Failed to parse raw transaction |
| `0xB006` | `SW_TX_HASH_FAIL`            | Failed to compute transaction hash |
| `0xB007` | `SW_BAD_STATE`               | Security issue with bad state |
| `0xB008` | `SW_SIGNATURE_FAIL`          | Failed to sign raw transaction |

### PLT Transaction Specific Errors

| Code   | Name                              | Description |
|--------|-----------------------------------|-------------|
| `0x6B0D` | `ERROR_PLT_CBOR_ERROR` | PLT CBOR parsing or processing error |
| `0x6B0E` | `ERROR_PLT_BUFFER_ERROR` | PLT buffer overflow or size error |
| `0x6B0F` | `ERROR_PLT_DATA_ERROR` | PLT data validation or integrity error |

## Error Handling Guidelines

1. When an error occurs, the application will:
   - Return the appropriate error code
   - Clear sensitive data from memory
   - Return to a safe state

2. User rejection errors (`ERROR_REJECTED_BY_USER`) are normal operations and indicate the user chose not to approve an action.

3. Device state errors may require user intervention (e.g., unlocking the device).

4. Buffer overflow errors indicate the client should break data into smaller chunks where applicable.



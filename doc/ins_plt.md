# Protocol Level Token (PLT)

A transaction type for handling protocol llevel token operations. The transaction includes a token ID and CBOR-encoded operation data.

## Protocol description

- Multiple commands with different chunk values to process the transaction in stages.

| INS    | P1     | P2     | CDATA                                                                                                                | Comment                                                                                |
| ------ | ------ | ------ | -------------------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------- |
| `0x27` | `0x01` | `0x00` | `path_length path[uint32]x[8] account_transaction_header[60 bytes] transaction_kind[uint8] token_id_length[uint8] token_id[1..255 bytes] cbor_length[uint32]` | Initial chunk (chunk=0). Transaction kind must be 27. |
| `0x27` | `0x01` | `0x00` | `cbor_data[1..900 bytes]`                                                                                            | Subsequent chunks. CBOR data can be sent in multiple chunks until complete. |

## Constraints

### Data Size Constraints
- Maximum token ID length: 255 bytes
- Maximum CBOR data length: 900 bytes per chunk
- CBOR data must be valid according to the CBOR specification
- Total display string length must not exceed 2000 bytes
### Operation Display Constraints
- **NBGL devices (Stax/Flex)**: Maximum 10 operations for individual display
- **BAGL devices (Nano X/S+)**: Maximum 5 operations for individual display  
- **JSON fallback**: When operation count exceeds device limits, display automatically falls back to JSON format
- **Display buffer limits**: 
  - **NBGL devices**: Use 32-pair display buffer system (2 pairs for header + ~3 pairs per operation)
  - **BAGL devices**: Use step-by-step display with memory constraints limiting to 5 operations

### Field Size Constraints
- **Operation type**: Limited by display buffer space
- **Amount values**: Must fit within display formatting constraints
- **Recipient addresses**: Must be valid Concordium addresses or account references
- **Target addresses**: Same constraints as recipient addresses

### Device-Specific Behavior
- **BAGL devices** (Nano S/S+/X): Use step-by-step display with automatic JSON fallback
- **NBGL devices** (Stax/Flex): Use structured display with pair-based UI, automatic JSON fallback

### Display Mode Selection
The app automatically chooses display mode based on operation count and device type:

**NBGL devices (Stax/Flex):**
- **≤ 10 operations**: Individual structured display showing each operation's fields
- **> 10 operations**: JSON format display of the complete transaction data

**BAGL devices (Nano X/S+):**
- **≤ 5 operations**: Individual structured display showing each operation's fields  
- **> 5 operations**: JSON format display of the complete transaction data

**All devices:**
- **Parse failures**: Automatic fallback to raw JSON display

## PLT-Specific Error Codes

In addition to the standard Concordium Ledger error codes, PLT transactions can return the following specific errors:

| Code   | Name                   | Description |
|--------|------------------------|-------------|
| `0x6B0D` | `ERROR_PLT_CBOR_ERROR` | PLT CBOR parsing or processing error |
| `0x6B0E` | `ERROR_PLT_BUFFER_ERROR` | PLT buffer overflow or size error |
| `0x6B0F` | `ERROR_PLT_DATA_ERROR` | PLT data validation or integrity error |

### When these errors occur:

- **ERROR_PLT_CBOR_ERROR**: CBOR decoding failed, tag parsing failed, hex formatting failed, or general CBOR processing issues
- **ERROR_PLT_BUFFER_ERROR**: Display buffer overflow, CBOR buffer too small, or chunk size exceeds available space
- **ERROR_PLT_DATA_ERROR**: Incomplete CBOR data, insufficient token/length data, or data integrity validation failures

## Error Handling

When a PLT-specific error occurs:
1. The transaction processing stops immediately
2. The appropriate error code is returned to the client
3. All sensitive data is cleared from memory
4. The device returns to a safe state
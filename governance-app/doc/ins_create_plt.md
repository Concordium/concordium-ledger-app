# Create PLT

An update instruction transaction for creating a new PLT (Programmable Liquidity Token) on the blockchain.

## Protocol description

* Multiple commands

INS | P1 | P2 | CDATA | Comment |
|----|--------|-----|-------------|----|
| `0x48` | `0x00` | `0x00` | `path_length path[uint32]x[5] update_instruction_header[28 bytes] update_type[uint8] payload_length[uint64]` | Update type must be 24. Contains derivation path and transaction header. |
| `0x48` | `0x01` | `0x00` | `token_symbol_length[uint32] [token_symbol[token_symbol_length bytes]] [token_module[32 bytes]] [decimals[uint8]] [initialization_params_length[uint32]]` | Transaction payload containing token details and initialization parameters length. All fields are included in this command. |
| `0x48` | `0x02` | `0x00` | `initialization_params[1...255 bytes]` | Initialization parameters bytes. Sent in batches until the entirety of the initialization parameters (`initialization_params_length` bytes) has been sent. This command is repeated until all initialization parameter data has been sent. |

## Data Format

The transaction payload is serialized in the following order according to Concordium serialization format:

1. **Token Symbol** (`String`): UTF-8 encoded string, maximum 128 characters
   - Serialized as: `[length: uint32][data: bytes]`

2. **Token Module** (`Hash`): 32-byte hash identifying the token module
   - Serialized as: `[32 bytes]`

3. **Decimals** (`uint8`): Number of decimal places for the token
   - Serialized as: `[1 byte]`

4. **Initialization Parameters** (`ByteArray`): Variable-length initialization data
   - Serialized as: `[length: uint32][data: bytes]`

## Transaction Flow

1. **Initial Command** (P1=0x00): Contains derivation path, update header, and payload length
2. **Payload Command** (P1=0x01): Contains all token details (symbol, module, decimals) **and initialization parameters length**
3. **Init Params Commands** (P1=0x02): Variable-length initialization data sent in 255-byte chunks

## Display Format

The Ledger will display the following information to the user:

- **Token Symbol**: The human-readable token identifier
- **Token Module**: Hex representation of the 32-byte module hash (paginated across multiple screens)
- **Decimals**: Number of decimal places
- **Init Params**: Hex representation of initialization parameters (paginated, up to 512 bytes displayed).

## Error Conditions

- `ERROR_INVALID_TRANSACTION`: Token symbol exceeds 128 characters
- `ERROR_INVALID_TRANSACTION`: Missing initialization parameters (length cannot be 0)
- `ERROR_INVALID_STATE`: Received more data than expected or invalid state transition
- `ERROR_INVALID_INSTRUCTION`: Incorrect update type (must be 24)

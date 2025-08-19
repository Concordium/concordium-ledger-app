# Update higher level keys

Consists of three types of transactions for updating root and level 1 keys:

* Update root keys using root keys
* Update level 1 keys using root keys
* Update level 1 keys using level 1 keys

## Protocol description

* Multiple commands
* INS should be `0x28` for root keys updating root keys, `0x29` for root keys updating level 1 keys, and `0x2B` for updating level 1
keys with level 1 keys.

INS | P1 | P2 | CDATA | Comment |
|----|--------|-----|-------------|----|
| `0x28/0x29/0x2B` | `0x00` | `0x00` | `path_length path[uint32]x[5] update_instruction_header[28 bytes] update_type[uint8] key_update_type[uint8] number_of_update_keys[uint16]` | |
| `0x28/0x29/0x2B` | `0x01` | `0x00` | `scheme_id[uint8] update_key[32 bytes]` | Repeated until `number_of_update_keys` update keys have been received. |
| `0x28/0x29/0x2B` | `0x02` | `0x00` | `threshold[uint16]` | The signing threshold for the new set of keys for the given key set. |

## Transaction Flow

1. **Review**: Shows a summary of the transaction.
2. **Update type**: Shows the human-readable update type (e.g., "Root keys update" or "Level 1 keys update").
3. **Key update type**: Shows which key level is being used to authorize the update.
4. **Number of keys**: Shows the number of keys being updated.
5. **Keys**: Shows each key (paginated if necessary).
6. **Threshold**: Shows the threshold for the new set of keys.
7. **Sign/Decline**: User can approve or reject the transaction.

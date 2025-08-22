# Update authorizations

Consists of two types of transactions for updating level 2 keys, i.e. the keys that are required to update chain parameters.

* Update level 2 keys using root keys
* Update level 2 keys using level 1 keys

Key update types:

* Key update type 2 is using level 1 keys and 3 is using root keys. Note that this is different from `V0` where 1 was level 1 keys and 2 was root keys.

P2 corresponds to the version of the authorizations targeted:
- 0 is used for V0
- 1 is used for V1
- 2 is used for V2

## Protocol description

* Multiple commands
* INS should be `0x2A` for updating with root keys, and `0x2C` for updating with level 1 keys.

INS | P1 | P2 | CDATA | Comment |
|----|--------|-----|-------------|----|
| `0x2A/0x2C` | `0x00` | `0x00/0x01/0x02` | `path_length path[uint32]x[5] update_instruction_header[28 bytes] update_type[uint8] key_update_type[uint8] number_of_update_keys[uint16]` | |
| `0x2A/0x2C` | `0x01` | `0x00/0x01/0x02` | `scheme_id[uint8] public_key[32 bytes]` | |
| `0x2A/0x2C` | `0x02` | `0x00/0x01/0x02` | `access_structure_size[uint16]` | The number of key indices for the current access structure. |
| `0x2A/0x2C` | `0x03` | `0x00/0x01/0x02` | `key_index[uint16]x[access_structure_size]` | Key indices for the current access structure. |
| `0x2A/0x2C` | `0x04` | `0x00/0x01/0x02` | `threshold[uint16]` | The signing threshold for the current access structure. If there are access structures that have not been transmitted, then GOTO command with `p1 == 0x02` and send the following access structure, and repeat until all access structures have been processed. |

## Transaction Flow

1. **Review**: Shows a summary of the transaction.
2. **Update type**: Shows the human-readable update type ("Update authorizations").
3. **Key update type**: Shows which key level is being used to authorize the update.
4. **Number of keys**: Shows the number of keys being updated.
5. **Keys**: Shows each key (paginated if necessary).
6. **Access structure**: Shows the access structure.
7. **Threshold**: Shows the threshold for the access structure.
8. **Sign/Decline**: User can approve or reject the transaction.

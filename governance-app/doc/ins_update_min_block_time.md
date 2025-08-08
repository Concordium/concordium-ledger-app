# Update minimum block time

Used for updating the minimum block time.

## Protocol description

* Single command

INS | P1 | P2 | CDATA | Comment |
|----|--------|-----|-------------|----|
| `0x44` | `0x00` | `0x00` | `path_length path[uint32]x[5] update_instruction_header[28 bytes] update_type[uint8] min_block_time[uint64]` | Update type must be 19.

## Transaction Flow

1. **Review**: Shows a summary of the transaction.
2. **Update type**: Shows the human-readable update type ("Min block time").
3. **Min block time**: Shows the minimum block time value.
4. **Sign/Decline**: User can approve or reject the transaction.

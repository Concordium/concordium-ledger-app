# Update block energy limit

Used for updating the block energy limit.

## Protocol description

* Single command

INS | P1 | P2 | CDATA | Comment |
|----|--------|-----|-------------|----|
| `0x45` | `0x00` | `0x00` | `path_length path[uint32]x[5] update_instruction_header[28 bytes] update_type[uint8] block_energy_limit[uint64]` | Update type must be 20.

## Transaction Flow

1. **Review**: Shows a summary of the transaction.
2. **Update type**: Shows the human-readable update type ("Block energy limit").
3. **Block energy limit**: Shows the new block energy limit.
4. **Sign/Decline**: User can approve or reject the transaction.

## Display Format

The Ledger will display the following information to the user:

- **Update type**: The human-readable update type (always shown after the review screen).
- **Block energy limit**: The new block energy limit value.

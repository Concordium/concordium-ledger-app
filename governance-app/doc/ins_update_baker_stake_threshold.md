# Update baker stake threshold

Updates the required baker stake threshold, i.e. the least required amount of stake to be able to bake.

## Protocol description

* Single command

INS | P1 | P2 | CDATA | Comment |
|----|--------|-----|-------------|----|
| `0x27` | `0x00` | `0x00` | `path_length path[uint32]x[5] update_instruction_header[28 bytes] update_type[uint8] baker_stake_threshold[uint64]` | The given baker_stake_threshold is supplied in µGTU. |

## Transaction Flow

1. **Review**: Shows a summary of the transaction.
2. **Update type**: Shows the human-readable update type ("Baker stake threshold").
3. **Stake threshold**: Shows the new baker stake threshold.
4. **Sign/Decline**: User can approve or reject the transaction.

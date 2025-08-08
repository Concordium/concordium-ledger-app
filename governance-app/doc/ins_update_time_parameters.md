# Update time parameters

Used for updating the reward period length and the mint rate per pay day.

## Protocol description

* Single command

INS | P1 | P2 | CDATA | Comment |
|----|--------|-----|-------------|----|
| `0x42` | `0x00` | `0x00` | `path_length path[uint32]x[5] update_instruction_header[28 bytes] update_type[uint8] reward_period_length[uint64] mint_rate_mantissa[uint32] mint_rate_exponent[uint8]` | the reward period length is provided in milliseconds |

## Transaction Flow

1. **Review**: Shows a summary of the transaction.
2. **Update type**: Shows the human-readable update type ("Time parameters").
3. **Reward Period Length**: Shows the reward period length.
4. **Mint rate**: Shows the mint rate.
5. **Sign/Decline**: User can approve or reject the transaction.

## Display Format

The Ledger will display the following information to the user:

- **Update type**: The human-readable update type (always shown after the review screen).
- **Reward Period Length**: The reward period length in milliseconds.
- **Mint rate**: The mint rate as a formatted string.

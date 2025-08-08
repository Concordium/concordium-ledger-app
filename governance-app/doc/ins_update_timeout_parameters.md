# Update timeout parameters

Used for updating the timeout parameters.

## Protocol description

* Single command

INS | P1 | P2 | CDATA | Comment |
|----|--------|-----|-------------|----|
| `0x43` | `0x00` | `0x00` | `path_length path[uint32]x[5] update_instruction_header[28 bytes] update_type[uint8] timeout_base[uint64] timeout_increase_numerator[uint64] timeout_increase_denominator[uint64] timeout_decrease_numerator[uint64] timeout_decrease_denominator[uint64]` | Update type must be 18. The timeout increase ratio must be greater than 1. The timeout decrease must be between 0 and 1. |

## Transaction Flow

1. **Review**: Shows a summary of the transaction.
2. **Update type**: Shows the human-readable update type ("Timeout parameters").
3. **Timeout base**: Shows the timeout base value.
4. **Increase ratio**: Shows the timeout increase ratio.
5. **Decrease ratio**: Shows the timeout decrease ratio.
6. **Sign/Decline**: User can approve or reject the transaction.

## Display Format

The Ledger will display the following information to the user:

- **Update type**: The human-readable update type (always shown after the review screen).
- **Timeout base**: The timeout base value.
- **Increase ratio**: The timeout increase ratio.
- **Decrease ratio**: The timeout decrease ratio.

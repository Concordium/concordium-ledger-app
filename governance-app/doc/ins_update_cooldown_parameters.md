# Update cooldown parameters

Used for updating the delegator cooldown and pool owner cooldown.

## Protocol description

* Single command

INS | P1 | P2 | CDATA | Comment |
|----|--------|-----|-------------|----|
| `0x40` | `0x00` | `0x00` | `path_length path[uint32]x[5] update_instruction_header[28 bytes] update_type[uint8] delegator_cooldown[uint64] pool_owner_cooldown[uint64]` | The cooldowns are provided in seconds |

## Transaction Flow

1. **Review**: Shows a summary of the transaction.
2. **Update type**: Shows the human-readable update type ("Cooldown parameters").
3. **Pool owner cooldown**: Shows the pool owner cooldown value.
4. **Delegator cooldown**: Shows the delegator cooldown value.
5. **Sign/Decline**: User can approve or reject the transaction.

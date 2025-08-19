# Update mint distribution

Used for updating the amount of minted GTU, and how the minted GTU is distributed.

Note that P2 should be 1, 0 is reserved for V0 of the transaction, which is no longer supported.

## Protocol description

* Single command

INS | P1 | P2 | CDATA | Comment |
|----|--------|-----|-------------|----|
| `0x25` | `0x00` | `0x01` | `path_length path[uint32]x[5] update_instruction_header[28 bytes] update_type[uint8] baker_reward_fraction[uint32] finalization_reward_fraction[uint32]` | The baker reward and finalization reward fractions are considered as fractions out of 100000. |

## Transaction Flow

1. **Review**: Shows a summary of the transaction.
2. **Update type**: Shows the human-readable update type ("Mint distribution").
3. **Baker reward**: Shows the baker reward fraction.
4. **Finalization reward**: Shows the finalization reward fraction.
5. **Sign/Decline**: User can approve or reject the transaction.

# Update GAS rewards

Used for updating the GAS rewards.

## Protocol description

* Single command

INS | P1 | P2 | CDATA | Comment |
|----|--------|-----|-------------|----|
| `0x23` | `0x00` | `0x00` | `path_length path[uint32]x[5] update_instruction_header[28 bytes] update_type[uint8] baker_reward[uint32] account_creation[uint32] chain_update[uint32]` | |

## Transaction Flow

1. **Review**: Shows a summary of the transaction.
2. **Update type**: Shows the human-readable update type ("GAS rewards").
3. **Baker**: Shows the baker reward.
4. **Account creation**: Shows the account creation reward.
5. **Chain update**: Shows the chain update reward.
6. **Sign/Decline**: User can approve or reject the transaction.

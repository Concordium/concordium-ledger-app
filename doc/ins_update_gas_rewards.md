# Update GAS rewards

Used for updating the GAS rewards.

## Protocol description

* Single command

INS | P1 | P2 | CDATA | Comment |
|----|--------|-----|-------------|----|
| `0x23` | `0x00` | `0x00` | `path_length path[uint32]x[5] update_instruction_header[28 bytes] update_type[uint8] baker_reward[uint32] finalization_proof[uint32] account_creation[uint32] chain_update[uint32]` | |

# Update mint distribution

Used for updating the amount of minted GTU, and how the minted GTU is distributed.

P2 is used to determine the version:
 - 0 for V0, which updates the mint rate per slot
 - 1 for V1, which does not update the mint rate

## Protocol description

* Single command

INS | P1 | P2 | CDATA | Comment |
|----|--------|-----|-------------|----|
| `0x25` | `0x00` | `0x00` | `path_length path[uint32]x[5] update_instruction_header[28 bytes] update_type[uint8] mint_rate_mantissa[uint32] mint_rate_exponent[uint8] baker_reward_fraction[uint32] finalization_reward_fraction[uint32]` | The baker reward and finalization reward fractions are considered as fractions out of 100000. |
| `0x25` | `0x00` | `0x01` | `path_length path[uint32]x[5] update_instruction_header[28 bytes] update_type[uint8] baker_reward_fraction[uint32] finalization_reward_fraction[uint32]` | The baker reward and finalization reward fractions are considered as fractions out of 100000. |

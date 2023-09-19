# Update time parameters

Used for updating the reward period length and the mint rate per pay day.

## Protocol description

* Single command

INS | P1 | P2 | CDATA | Comment |
|----|--------|-----|-------------|----|
| `0x42` | `0x00` | `0x00` | `path_length path[uint32]x[5] update_instruction_header[28 bytes] update_type[uint8] reward_period_length[uint64] mint_rate_mantissa[uint32] mint_rate_exponent[uint8]` | the reward period length is provided in milliseconds |

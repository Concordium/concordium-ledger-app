# Update timeout parameters

Used for updating the timeout parameters.

## Protocol description

* Single command

INS | P1 | P2 | CDATA | Comment |
|----|--------|-----|-------------|----|
| `0x43` | `0x00` | `0x00` | `path_length path[uint32]x[5] update_instruction_header[28 bytes] update_type[uint8] timeout_base[uint64] timeout_increase_numerator[uint64] timeout_increase_denominator[uint64] timeout_decrease_numerator[uint64] timeout_decrease_denominator[uint64]` | Update type must be 18. The timeout increase ratio must be greater than 1. The timeout decrease must be between 0 and 1. |

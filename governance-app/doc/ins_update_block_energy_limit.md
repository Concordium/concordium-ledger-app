# Update block energy limit

Used for updating the block energy limit.

## Protocol description

* Single command

INS | P1 | P2 | CDATA | Comment |
|----|--------|-----|-------------|----|
| `0x45` | `0x00` | `0x00` | `path_length path[uint32]x[5] update_instruction_header[28 bytes] update_type[uint8] block_energy_limit[uint64]` | Update type must be 20.

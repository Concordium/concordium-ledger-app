# Update validator score parameters

Used for updating the parameters determining the "score" of a validator which are used when deciding whether a validator should be suspended.

## Protocol description

* Single command

INS | P1 | P2 | CDATA | Comment |
|----|--------|-----|-------------|----|
| `0x47` | `0x00` | `0x00` | `path_length path[uint32]x[5] update_instruction_header[28 bytes] update_type[uint8] max_missed_rounds[uint64]` | `update_type` must be `23`. |

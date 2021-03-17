# Transfer transaction

A transaction to update the transaction fee distribution chain parameter.

## Protocol description

* Single command

| P1 | P2 | CDATA | Comment |
|--------|-----|-------------|----|
| `0x00` | `0x00` | `path_length path[uint32]x[8] update_instruction_header[28 bytes] update_type[uint8] baker_fee[uint32] gas_account_fee[uint32]` | Update type must be 7 |

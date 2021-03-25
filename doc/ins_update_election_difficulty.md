# Update foundation account

Used for updating the account address of the foundation account, i.e. updating which account is considered the foundation account that will 
receive foundation rewards.

## Protocol description

* Single command

INS | P1 | P2 | CDATA | Comment |
|----|--------|-----|-------------|----|
| `0x26` | `0x00` | `0x00` | `path_length path[uint32]x[5] update_instruction_header[28 bytes] update_type[uint8] election_difficulty[uint32]` | The update type has to be 2. |

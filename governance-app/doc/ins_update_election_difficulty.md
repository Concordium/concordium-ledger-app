# Update election difficulty

Used for updating the election difficulty. The election difficulty is a fraction out of 100000, i.e. the transaction
determines the numerator in `election_difficulty/100000`.

## Protocol description

* Single command

INS | P1 | P2 | CDATA | Comment |
|----|--------|-----|-------------|----|
| `0x26` | `0x00` | `0x00` | `path_length path[uint32]x[5] update_instruction_header[28 bytes] update_type[uint8] election_difficulty[uint32]` | The update type has to be 2. |

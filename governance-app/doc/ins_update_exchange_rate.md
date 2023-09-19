# Update exchange rate

This instruction supports signing two separate update transaction types. A single instruction covers both transactions as the
payload body of both transactions are identical.

1. Update euro per energy
1. Update µGTU per Euro

## Protocol description

* Single command

INS | P1 | P2 | CDATA | Comment |
|----|--------|-----|-------------|----|
| `0x06` | `0x00` | `0x00` | `path_length path[uint32]x[5] update_instruction_header[28 bytes] update_type[uint8] exchange_rate_numerator[uint64] exchange_rate_denominator[uint64]` | Update type must be 3 (euro per energy) or 4 (µGTU per euro). |

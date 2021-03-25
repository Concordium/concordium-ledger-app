# Add baker

Used to add baker keys to an account.

## Protocol description

* Mutiple commands

INS | P1 | P2 | CDATA | Comment |
|----|--------|-----|-------------|----|
| `0x13` | `0x00` | `0x01` | `path[uint32]x[8] account_transaction_header[60 bytes] transaction_kind[uint8]` | |
| `0x13` | `0x01` | `0x01` | `election_verify_key[32 bytes] baker_sign_verify_key[32 bytes] baker_aggregation_verify_key[96 bytes]` | |
| `0x13` | `0x02` | `0x01` | `proofs[192 bytes]` | |

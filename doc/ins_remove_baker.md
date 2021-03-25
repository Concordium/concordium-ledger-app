# Remove baker

Used to remove the baker for the account that submits the transaction.

## Protocol description

* Single command

INS | P1 | P2 | CDATA | Comment |
|----|--------|-----|-------------|----|
| `0x14` | `0x00` | `0x00` | `path[uint32]x[8] account_transaction_header[60 bytes] transaction_kind[uint8]` | |

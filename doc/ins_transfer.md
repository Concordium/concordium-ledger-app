# Transfer transaction

A transaction to transfer GTU from one account to another.

## Protocol description

* Single command

INS | P1 | P2 | CDATA | Comment |
|----|--------|-----|-------------|----|
| `0x02` | `0x00` | `0x00` | `path_length path[uint32]x[8] account_transaction_header[60 bytes] transaction_kind[uint8] recipient_address[32 bytes] amount[uint64]` | The amount is in µGTU. The recipient address has to be base58. |

# Transfer to encrypted

A transaction for transferring an amount to the encrypted balance of the account.

## Protocol description

* Single command

INS | P1 | P2 | CDATA | Comment |
|----|--------|-----|-------------|----|
| `0x11` | `0x00` | `0x00` | `path_length path[uint32]x[8] account_transaction_header[60 bytes] transaction_kind[uint8] amount_to_encrypted[uint64]` | |

# Encrypted amount transfer

A transaction for transferring an encrypted amount to another account.

## Protocol description

* Multiple commands.

INS | P1 | P2 | CDATA | Comment |
|----|--------|-----|-------------|----|
| `0x11` | `0x00` | `0x00` | `path_length path[uint32]x[8] account_transaction_header[60 bytes] transaction_kind[uint8] to_address[32 bytes] remaining_amount transfer_amount index encrypted_amount_agg_index proofs_size[uint16]` | |
| `0x11` | `0x01` | `0x00` | `proofs[1..255 bytes]` | In batches of up to 255 bytes, repeated until all proofs have been sent. |

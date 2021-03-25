# Transfer to public

A transaction for transferring an encrypted amount to the public balance of the account.

## Protocol description

* Multiple commands.

INS | P1 | CDATA | Comment |
|---|--------|-------------|----|
| `0x12` | `0x00` | `path_length path[uint32]x[8] account_transaction_header[60 bytes] transaction_kind[uint8] remaining_encrypted_amount[192 bytes] transfer_amount[uint64] encrypted_amount_agg_index[8 bytes] proofs_size[uint16]` | |
| `0x12` | `0x01` | `proofs[1..255 bytes]` | In batches of up to 255 bytes, repeated until all proofs have been sent. |

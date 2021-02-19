# Transfer to public

A transaction for transferring an encrypted amount to the public balance of the account.

## Protocol description

* Multiple commands.

INS | P1 | CDATA | Comment |
|---|--------|-------------|----|
| `0x12` | `0x00` | `path_length path transaction_header transaction_kind remaining_encrypted_amount transfer_amount encrypted_amount_agg_index proofs_size` | |
| `0x12` | `0x01` | `proofs` | In batches of up to 255 bytes. |

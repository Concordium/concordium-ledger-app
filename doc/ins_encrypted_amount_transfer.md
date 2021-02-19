# Encrypted amount transfer

A transaction for transferring an encrypted amount to another account.

## Protocol description

* Single command.

INS | P1 | CDATA | Comment |
|---|--------|-------------|----|
| `0x11` | `0x00` | `path_length path transaction_header transaction_kind to_address remaining_amount transfer_amount index encrypted_amount_agg_index proofs_size` | |
| `0x11` | `0x01` | `proofs` | In batches of up to 255 bytes. |

# Update baker restake earnings

Used to update whether to restake baker earnings or not.

## Protocol description

* Single command

| INS | P1 | P2 | CDATA | Comment |
|---|--------|-----|-------------|----|
| `0x16` | `0x00` | `0x00` | `path[uint32]x[8] account_transaction_header[60 bytes] transaction_kind[uint8] restake_earnings[uint8]` | Restake earnings has to be 0 (do not restake) or 1 (restake). Transaction kind must be 7. |

# Transfer transaction

A transaction to transfer GTU from one account to another.

## Protocol description

- Single command

| INS    | P1     | P2     | CDATA                                                                                                                                  | Comment                                                        |
| ------ | ------ | ------ | -------------------------------------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------- |
| `0x02` | `0x00` | `0x00` | `path_length path[uint32]x[8] account_transaction_header[60 bytes] transaction_kind[uint8] recipient_address[32 bytes] amount[uint64]` | The amount is in µGTU. The recipient address has to be base58. |

# Transfer with memo

A transaction to transfer GTU from one account to another, with a memo attached.
Uses the same INS number, but a different P1 for the initial call, and has a different transaction kind (22);

## Protocol description

- Multiple commands

| INS    | P1     | P2     | CDATA                                                                                                                                       | Comment                                 |
| ------ | ------ | ------ | ------------------------------------------------------------------------------------------------------------------------------------------- | --------------------------------------- |
| `0x02` | `0x01` | `0x00` | `path_length path[uint32]x[8] account_transaction_header[60 bytes] transaction_kind[uint8] recipient_address[32 bytes] memo_length[uint16]` | The recipient address has to be base58. |
| `0x02` | `0x02` | `0x00` | `memo[1...255 bytes]`                                                                                                                       | The memo is assumed to be CBOR encoded. |
| `0x02` | `0x03` | `0x00` | `amount[uint64]`                                                                                                                            | The amount is in µGTU.                  |

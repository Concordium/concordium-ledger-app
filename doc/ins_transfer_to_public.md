# Transfer to public

A transaction for transferring an encrypted amount to the public balance of the account.

## Protocol description

- Multiple commands with different P1 values to process the transaction in stages.

| INS    | P1     | P2     | CDATA                                                                                                                                               | Comment                                                |
| ------ | ------ | ------ | --------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------ |
| `0x12` | `0x00` | `0x00` | `path_length path[uint32]x[8] account_transaction_header[60 bytes] transaction_kind[uint8]`                                                         | Initial command to process path and transaction header |
| `0x12` | `0x01` | `0x00` | `remaining_encrypted_amount[192 bytes] transfer_amount[uint64] recipient_address[32 bytes] encrypted_amount_agg_index[8 bytes] proofs_size[uint16]` | Command to process transfer details                    |
| `0x12` | `0x02` | `0x00` | `proofs[1..255 bytes]`                                                                                                                              | Process proof data in batches until all proofs sent    |

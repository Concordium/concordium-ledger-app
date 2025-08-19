# Transfer transaction

A transaction to update the transaction fee distribution chain parameter.

## Protocol description

* Single command

| INS | P1 | P2 | CDATA | Comment |
|---|--------|-----|-------------|----|
| `0x22` | `0x00` | `0x00` | `path_length path[uint32]x[8] update_instruction_header[28 bytes] update_type[uint8] baker_fee[uint32] gas_account_fee[uint32]` | Update type must be 7 |

## Transaction Flow

1. **Review**: Shows a summary of the transaction.
2. **Update type**: Shows the human-readable update type ("Transaction fee distribution").
3. **Baker fee**: Shows the baker fee.
4. **GAS account fee**: Shows the GAS account fee.
5. **Sign/Decline**: User can approve or reject the transaction.

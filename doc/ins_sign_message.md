# Sign message

Sign an arbitrary message with the private key of the account.

What is signed is the sha 256 hash of the account address (32 bytes) || 8 zero bytes || the message bytes. 

The message is displayed as UTF8, unless the initial command uses P2 = 1, in which case the message will be displayed using hex encoding instead.

## Protocol description

* Multiple commands

INS | P1 | P2 | CDATA | Comment |
|----|--------|-----|-------------|----|
| `0x38` | `0x00` | `0x00` / `0x01` | `path_length path[uint32]x[8] signer_address[32 bytes] message_length[uint16]` | The signer address has to be base58. |
| `0x38` | `0x01` | `0x00` | `message[1...255 bytes]` | The message should be sent in batches of up to 255 bytes. |

# Update protocol

A transaction for updating the protocol.

## Protocol description

* Multiple commands.

INS | P1 | CDATA | Comment |
|---|--------|-------------|----|
| `0x21` | `0x00` | `path_length path update_header update_type payload_length` | |
| `0x21` | `0x01` | `text_length` | First message_length and then specification_url_length |
| `0x21` | `0x02` | `text` | First message_text and then specification_text. Up to 255 bytes per command. |
| `0x21` | `0x03` | `specification_hash` | |
| `0x21` | `0x04` | `auxiliary_data` | Up to 255 bytes per command until all has been sent. The number of bytes to be received is exactly the `payload_length` minus the rest of the transaction bytes. |

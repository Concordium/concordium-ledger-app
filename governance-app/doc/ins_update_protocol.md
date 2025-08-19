# Update protocol

An update instruction transaction for updating the blockchain protocol.

## Protocol description

* Multiple commands

INS | P1 | P2 | CDATA | Comment |
|----|--------|-----|-------------|----|
| `0x21` | `0x00` | `0x00` | `path_length path[uint32]x[5] update_instruction_header[28 bytes] update_type[uint8] payload_length[uint64]` | Update type must be 1. |
| `0x21` | `0x01` | `0x00` | `message_length[uint64]` | The length of the incoming message bytes. Maximum of 255 with the current implementation, but more could be supported. |
| `0x21` | `0x02` | `0x00` | `message[message_length_bytes]` | The message as UTF-8 encoded bytes. |
| `0x21` | `0x01` | `0x00` | `specification_url_length[uint64]` | The length of the incoming URL specification text. Maximum of 255 with the current implementation, but more could be supported. |
| `0x21` | `0x02` | `0x00` | `specification_url[specification_url_length_bytes]` | The specification URL as UTF-8 encoded bytes. |
| `0x21` | `0x03` | `0x00` | `specification_hash[32 bytes]` | |
| `0x21` | `0x04` | `0x00` | `auxiliary_data[1..255 bytes]` | Auxiliary data. This command is repeated until all the auxiliary data has been sent. The application tracks this by doing a calculation based on payload_length received in the initial packet. |

## Transaction Flow

1. **Review**: Shows a summary of the transaction.
2. **Update type**: Shows the human-readable update type ("Protocol update").
3. **Message**: Shows the message accompanying the update.
4. **Specification URL**: Shows the URL for the protocol specification.
5. **Specification hash**: Shows the hash of the specification.
6. **Auxiliary data**: Shows the auxiliary data if present.
7. **Sign/Decline**: User can approve or reject the transaction.

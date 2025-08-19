# Add Identity Provider

An update instruction transaction for adding an identity provider to the blockchain.

## Protocol description

* Multiple commands

INS | P1 | P2 | CDATA | Comment |
|--------|--------|--------|--------------------------------------------------------------------------------------------------------------|-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| `0x2D` | `0x00` | `0x00` | `path_length path[uint32]x[5] update_instruction_header[28 bytes] update_type[uint8] payload_length[uint64] IpInfo_length[uint32] IpIdentity[uint32] ` | Update type must be 13.                                                                                                                                                                          |
| `0x2D` | `0x01` | `0x00` | `description_length[uint32]`                                                                                     | The length (`[uint32]`) of the incoming part of the description structure.  (Either `name`, `url` or `description`)                                                                          |
| `0x2D` | `0x02` | `0x00` | `description[1...255 bytes]`                                                                              | The bytes of a part of description (Either `name`, `url` or `description`). Sent in batches until the entirety of the current part (`[description_length bytes]`) has been sent.                                                                                                                                                             |
| `0x2D` | `0x03` | `0x00` | `verify_key[verify_key_length_bytes]`                                                          | The verify key bytes.                                                                                                                                                   |
| `0x2D` | `0x04` | `0x00` | `cdi_verify_key[32 bytes]`                                                                               |                                                                                                                                                                                                 |

## Transaction Flow

1. **Review**: Shows a summary of the transaction.
2. **Update type**: Shows the human-readable update type ("Add identity provider").
3. **Identity provider**: Shows the identity provider.
4. **Name**: Shows the name field.
5. **URL**: Shows the URL field.
6. **Description**: Shows the description field.
7. **Verify Key Hash**: Shows the verify key hash.
8. **CDI Verify key**: Shows the CDI verify key.
9. **Sign/Decline**: User can approve or reject the transaction.

# Add Anonymity Revoker

An update instruction transaction for adding an anonymity revoker to the blockchain.

## Protocol description

* Multiple commands

INS | P1 | P2 | CDATA | Comment |
|--------|--------|--------|--------------------------------------------------------------------------------------------------------------|-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| `0x2C` | `0x00` | `0x00` | `path_length path[uint32]x[5] update_instruction_header[28 bytes] update_type[uint8] payload_length[uint64] ArInfo_length[uint32] ArIdentity[uint32] ` | Update type must be 12.                                                                                                                                                                          |
| `0x2C` | `0x01` | `0x00` | `description_length[uint32]`                                                                                     | The length (`[uint32]`) of the incoming part of the description structure.  (Either `name`, `url` or `description`)                                                                          |
| `0x2C` | `0x02` | `0x00` | `description[1...255 bytes]`                                                                              | The bytes of a part of description (Either `name`, `url` or `description`). Sent in batches until the entirety of the current part (`[description_length bytes]`) has been sent.                                                                                                                                                             |
| `0x2C` | `0x03` | `0x00` | `public key[96 bytes]`                                                                               |                                                                                                                                                                                                 |

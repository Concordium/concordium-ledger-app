# Add Identity Provider

An update instruction transaction for adding an identity provider to the blockchain.

## Protocol description

* Multiple commands

INS | P1 | P2 | CDATA | Comment |
|--------|--------|--------|--------------------------------------------------------------------------------------------------------------|-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| `0x2D` | `0x00` | `0x00` | `path_length path[uint32]x[5] update_instruction_header[28 bytes] update_type[uint8] payload_length[uint64] IpInfo_length[uint32] IpIdentity[uint32] ` | Update type must be 13.                                                                                                                                                                          |
| `0x2D` | `0x01` | `0x00` | `description_length[uint32]`                                                                                     | The length of the incoming description structure.                                                                          |
| `0x2D` | `0x02` | `0x00` | `description[description_length_bytes]`                                                                              | The description bytes (could be split into `name_length[uint32]`,`name`,`url_length[uint32]`,`url`,`description_length[uint32]`,`description`)                                                                                                                                                             |
| `0x2D` | `0x03` | `0x00` | `verify_key_length[uint32]`                                                                           | The length of the verify key.                                                                 |
| `0x2D` | `0x04` | `0x00` | `verify_key[verify_key_length_bytes]`                                                          | The verify key bytes.                                                                                                                                                   |
| `0x2D` | `0x05` | `0x00` | `cdi_verify_key[32 bytes]`                                                                               |                                                                                                                                                                                                 |

# Add Anonymity Revoker

An update instruction transaction for adding an anonymity revoker to the blockchain.

## Protocol description

* Multiple commands

INS | P1 | P2 | CDATA | Comment |
|--------|--------|--------|--------------------------------------------------------------------------------------------------------------|-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| `0x2D` | `0x00` | `0x00` | `path_length path[uint32]x[5] update_instruction_header[28 bytes] update_type[uint8] payload_length[uint64] ArInfo_length[uint32] ArIdentity[uint32] ` | Update type must be 12.                                                                                                                                                                          |
| `0x2D` | `0x01` | `0x00` | `description_length[uint32]`                                                                                     | The length of the incoming description structure.                                                                          |
| `0x2D` | `0x02` | `0x00` | `description[description_length_bytes]`                                                                              | The description bytes (could be split into `name_length[uint32]`,`name`,`url_length[uint32]`,`url`,`description_length[uint32]`,`description`)                                                                                                                                                             |
| `0x2D` | `0x05` | `0x00` | `public key[96 bytes]`                                                                               |                                                                                                                                                                                                 |

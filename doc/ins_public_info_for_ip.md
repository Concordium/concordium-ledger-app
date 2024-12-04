# Public information for identity provider

When creating an identity some data has to be signed as a part of the protocol for identity creation, which
is carried out with the identity provider. This function allows for the signing of that data.

## Protocol description

- Multiple commands

| INS    | P1     | P2     | CDATA                                                                                          | Comment                                                               |
| ------ | ------ | ------ | ---------------------------------------------------------------------------------------------- | --------------------------------------------------------------------- |
| `0x20` | `0x00` | `0x00` | `path_length path[uint32]x[8] id_cred_pub[48 bytes] cred_id[48 bytes] public_key_count[uint8]` |                                                                       |
| `0x20` | `0x01` | `0x00` | `key_index[uint8] key_type[uint8] public_key[32 bytes]`                                        | Instruction is repeated until `public_key_count` keys have been sent. |
| `0x20` | `0x02` | `0x00` | `threshold[uint8]`                                                                             |                                                                       |

# Export public key

Provides the ability to export public-keys for accounts, and also for governance keys. The key to
be exported is defined by the key derivation path that is provided, which can either be for an
account or for a governance key.

## Protocol description

- Single command

| INS    | P1     | P2     | CDATA                 | Comment                                                |
| ------ | ------ | ------ | --------------------- | ------------------------------------------------------ |
| `0x01` | `0x00` | `0x00` | `path[uint32]x[5..8]` | Export of a public-key with user acceptance.           |
| `0x01` | `0x01` | `0x00` | `path[uint32]x[5..8]` | Export of a public-key without user acceptance.        |
| `0x01` | `0x00` | `0x01` | `path[uint32]x[5..8]` | Export of a signed public-key with user acceptance.    |
| `0x01` | `0x01` | `0x01` | `path[uint32]x[5..8]` | Export of a signed public-key without user acceptance. |

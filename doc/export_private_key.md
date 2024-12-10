# Export private key

As the Ledger Nano S (at the time of writing) does not support the necessary key types, we have implemented an export
of some special private keys. These keys are not account signing keys, and note that it is not possible to export keys that are used for signatures.
Signature keys do not leave the device at any point, and the exported keys cannot be used to submit transactions.

The two key types that can be exported are exactly these:

1. PRF-key
1. IdCredSec

If P2 = 0x01, the exported keys are derived using SLIP10 for ed25519.
These must be used as the key seed for the KeyGen algorithm for generating BLS12-381 private keys.
This endpoint is deprecated and should not be used. (Available to support legacy accounts from the desktop wallet)

If P2 = 0x02, the BLS12-381 private keys will be exported instead. (Generated using the corresponding ed25519 key as key seed)

## Protocol description

For legacy paths:

| INS    | P1     | P2     | CDATA                   | Comment                                                                                                        |
| ------ | ------ | ------ | ----------------------- | -------------------------------------------------------------------------------------------------------------- |
| `0x05` | `0x00` | `0x01` | `0x00,identity[uint32]` | Export of PRF key seed for the BLS12-381 KeyGen algorithm (Deprecated)                                         |
| `0x05` | `0x01` | `0x01` | `0x00,identity[uint32]` | Export of PRF key seed for the BLS12-381 KeyGen algorithm with alternative display (for recovery) (Deprecated) |
| `0x05` | `0x02` | `0x01` | `0x00,identity[uint32]` | Export of PRF key and IdCredSec seeds for the BLS12-381 KeyGen algorithm (Deprecated)                          |
| `0x05` | `0x00` | `0x02` | `0x00,identity[uint32]` | Export of PRF key (BLS12-381)                                                                                  |
| `0x05` | `0x01` | `0x02` | `0x00,identity[uint32]` | Export of PRF key with alternative display (for recovery) (BLS12-381)                                          |
| `0x05` | `0x02` | `0x02` | `0x00,identity[uint32]` | Export of PRF key and IdCredSec (BLS12-381)                                                                    |

For new paths:

| INS    | P1     | P2     | CDATA                                             | Comment                                                                                                        |
| ------ | ------ | ------ | ------------------------------------------------- | -------------------------------------------------------------------------------------------------------------- |
| `0x05` | `0x00` | `0x01` | `0x01,identity_provider[uint32],identity[uint32]` | Export of PRF key seed for the BLS12-381 KeyGen algorithm (Deprecated)                                         |
| `0x05` | `0x01` | `0x01` | `0x01,identity_provider[uint32],identity[uint32]` | Export of PRF key seed for the BLS12-381 KeyGen algorithm with alternative display (for recovery) (Deprecated) |
| `0x05` | `0x02` | `0x01` | `0x01,identity_provider[uint32],identity[uint32]` | Export of PRF key and IdCredSec seeds for the BLS12-381 KeyGen algorithm (Deprecated)                          |
| `0x05` | `0x00` | `0x02` | `0x01,identity_provider[uint32],identity[uint32]` | Export of PRF key (BLS12-381)                                                                                  |
| `0x05` | `0x01` | `0x02` | `0x01,identity_provider[uint32],identity[uint32]` | Export of PRF key with alternative display (for recovery) (BLS12-381)                                          |
| `0x05` | `0x02` | `0x02` | `0x01,identity_provider[uint32],identity[uint32]` | Export of PRF key and IdCredSec (BLS12-381)                                                                    |

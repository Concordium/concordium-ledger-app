# Export private key seed

As the Ledger Nano S (at the time of writing) does not support the necessary key types, we have implemented an export
of some special private keys that are used as key seeds outside of the device. These keys are not account signing keys,
and note that it is not possible to export keys that are used for signatures. Signature keys do not leave the device
at any point, and the exported keys cannot be used to submit transactions.

The two key types that can be exported are exactly these:

1. PRF-key
1. IdCredSec

The exported keys are derived using SLIP10 for ed25519, and they will serve as the key seed for the KeyGen algorithm
for generating BLS12-381 private keys.

## Protocol description

* Single command

| P1 | P2 | CDATA | Comment |
|--------|--------|------------|----|
| `0x00` | `0x01` | `identity[uint32]` | Export of PRF key |
| `0x01` | `0x01` | `identity[uint32]` | Export of PRF key with alternative display (for recovery) |
| `0x02` | `0x01` | `identity[uint32]` | Export of PRF key and IdCredSec |

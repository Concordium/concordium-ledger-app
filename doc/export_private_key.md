# Export private key

The exported private keys are not account signing keys, and note that it is not possible to export keys that are used for signatures.
Signature keys do not leave the device at any point, and the exported keys cannot be used to submit transactions.

## Legacy Path (INS 0x05)

The instruction code (INS) for legacy export private key is `0x05`.

The two key types that can be exported for the Legacy Path are exactly these:

1. PRF-key
2. IdCredSec

If P2 = 0x01, the exported keys are derived using SLIP10 for ed25519.
These must be used as the key seed for the KeyGen algorithm for generating BLS12-381 private keys.
This endpoint is deprecated and should not be used. (Available to support legacy accounts from the desktop wallet)

If P2 = 0x02, the BLS12-381 private keys will be exported instead. (Generated using the corresponding ed25519 key as key seed)

### Legacy Path Protocol Description

| INS    | P1     | P2     | CDATA              | Comment                                                                                                        |
| ------ | ------ | ------ | ------------------ | -------------------------------------------------------------------------------------------------------------- |
| `0x05` | `0x00` | `0x01` | `identity[uint32]` | Export of PRF key seed for the BLS12-381 KeyGen algorithm (Deprecated)                                         |
| `0x05` | `0x01` | `0x01` | `identity[uint32]` | Export of PRF key seed for the BLS12-381 KeyGen algorithm with alternative display (for recovery) (Deprecated) |
| `0x05` | `0x02` | `0x01` | `identity[uint32]` | Export of PRF key and IdCredSec seeds for the BLS12-381 KeyGen algorithm (Deprecated)                          |
| `0x05` | `0x00` | `0x02` | `identity[uint32]` | Export of PRF key (BLS12-381)                                                                                  |
| `0x05` | `0x01` | `0x02` | `identity[uint32]` | Export of PRF key with alternative display (for recovery) (BLS12-381)                                          |
| `0x05` | `0x02` | `0x02` | `identity[uint32]` | Export of PRF key and IdCredSec (BLS12-381)                                                                    |

## New Path (INS 0x37)

The instruction code (INS) for new export private key is `0x37`.

The new path supports more diverse key export scenarios for different use cases in the Concordium ecosystem. Keys cannot be exported individually but rather per purpose.

### New Path Protocol Description

| INS    | P1     | P2     | CDATA                                                              | Comment                                                                                                   |
| ------ | ------ | ------ | ------------------------------------------------------------------ | --------------------------------------------------------------------------------------------------------- |
| `0x37` | `0x00` | `0x00` | `identity_provider[uint32],identity[uint32]`                       | Identity Credential Creation: Export IDCredSec (BLS) + PRFKey (BLS) + Signature Blinding Randomness (BLS) |
| `0x37` | `0x01` | `0x00` | `identity_provider[uint32],identity[uint32],account_index[uint32]` | Account Creation: Export PRFKey (BLS) + IDCredSec (BLS) + Commitment Randomness root (Ed25519)            |
| `0x37` | `0x02` | `0x00` | `identity_provider[uint32],identity[uint32]`                       | ID Recovery: Export IDCredSec (BLS) + Signature Blinding Randomness (BLS)                                 |
| `0x37` | `0x03` | `0x00` | `identity_provider[uint32],identity[uint32]`                       | Account Credential Discovery: Export PRFKey (BLS)                                                         |
| `0x37` | `0x04` | `0x00` | `identity_provider[uint32],identity[uint32],account_index[uint32]` | Zero-knowledge proofs: Export Commitment Randomness (Ed25519)                                             |

### Key Derivation Paths

#### Identity Credential Creation (P1=0x00)

- IDCredSec: `m/44'/919'/{IDP}'/{ID}'/2'` (BLS field element)
- PRFKey: `m/44'/919'/{IDP}'/{ID}'/3'` (BLS field element)
- Signature Blinding Randomness: `m/44'/919'/{IDP}'/{ID}'/4'` (BLS field element)

#### Account Creation (P1=0x01)

- PRFKey: `m/44'/919'/{IDP}'/{ID}'/3'` (BLS field element)
- IDCredSec: `m/44'/919'/{IDP}'/{ID}'/2'` (BLS field element)
- Commitment Randomness: `m/44'/919'/{IDP}'/{ID}'/5'/{Account Credential Index}'` (Ed25519)

#### ID Recovery (P1=0x02)

- IDCredSec: `m/44'/919'/{IDP}'/{ID}'/2'` (BLS field element)
- Signature Blinding Randomness: `m/44'/919'/{IDP}'/{ID}'/4'` (BLS field element)

#### Account Credential Discovery (P1=0x03)

- PRFKey: `m/44'/919'/{IDP}'/{ID}'/3'` (BLS field element)

#### Zero-knowledge proofs (P1=0x04)

- Commitment Randomness: `m/44'/919'/{IDP}'/{ID}'/5'/{Account Credential Index}'` (Ed25519)

### Key Export Format

For new paths, the output format is `[length of key][key]` and repeats for each key exported. Keys are exported in the order specified above.

#### Key Types

- **IDCredSec**: BLS field element
- **PRFKey**: BLS field element
- **Signature Blinding Randomness**: BLS field element
- **Commitment Randomness**: Ed25519 private key (allows SLIP-10 derivation of child keys)

# Export data

This method allows exporting various data for the identity. This data does not include account signing keys, and note that it is not possible to export keys that are used for signatures. 
Signature keys do not leave the device at any point, and the exported data, which includes some types of keys, cannot be used to submit transactions.

The data types that can be exported are the following:

1. PRF-key
1. IdCredSec
2. Signature retrieval / blinding randomness
3. Attribute randomness

The initial command uses P1 to decide what is exported:

P1 = 0x00: Export the PRF-key, idCredSec and the signature retrieval randomness, but no attribute randomness.
P1 = 0x01: Export the PRF-key, idCredSec and the signature retrieval randomness and randomness for the specified attribute tags.
P1 = 0x02: Export only randomness for the specified attribute tags.

## Protocol description

* Single command

| INS | P1 | P2 | CDATA | Comment |
|-----|--------|--------|------------|----|
| `0x07` | `0x00` | `0x00` | `coin_type[uint32] identity_provider[uint32] identity[uint32]` | Exports the PRF key and IdCredSec and blinding randomness (BLS12-381) |

* Multi command

| INS | P1 | P2 | CDATA | Comment |
|-----|--------|--------|------------|----|
| `0x07` | `0x01` or `0x02` | `0x00` | `coin_type[uint32] identity_provider[uint32] identity[uint32] credential_counter[uint32] attributeCount[uint16] attribute_tag[attributeCount bytes]` | Exports the PRF key and IdCredSec and blinding randomness if p1 = 0x02,  maximum attributeCount is 235 |
| `0x07` | `0x03` | `0x00` |  | Exports attribute randomness in batches of 7 until the randomness has been returned for each requested attribute_tag. Note that the final batch will contain the randomness for (attributeCount % 7) attributes |

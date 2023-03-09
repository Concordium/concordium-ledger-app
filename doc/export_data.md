# Export data

This method allows exporting various data for the identity. This data does not include not account signing keys, and note that it is not possible to export keys that are used for signatures. 
Signature keys do not leave the device at any point, and the exported data, which includes some types of keys, cannot be used to submit transactions.

The data types that can be exported are the following, and which P1 is used for that data:

1. PRF-key                                   - 0x02
1. IdCredSec                                 - 0x03
2. Signature retrieval / blinding randomness - 0x04
3. Attribute randomness                      - 0x05

If P1 = 0x00, then the if user accepts, all data (P1 \in [2-5]) can be exported for the same P2, identity and identity provider without prompting the user. Note that the instruction should be repeated with P1 = 0x00 to finish the session.
Note that P1 = 0x01 is not used/allowed.

if P2 = 0x00, then the Mainnet path/coinType is used, if P2 = 0x01 then the Testnet path/coinType is used instead.

## Protocol description

* Single command

| INS | P1 | P2 | CDATA | Comment |
|-----|--------|--------|------------|----|
| `0x07` | `0x02` | `0x00` or `0x01` | `identity_provider[uint32] identity[uint32]` | Export of PRF key (BLS12-381) |
| `0x07` | `0x03` | `0x00` or `0x01` | `identity_provider[uint32] identity[uint32]` | Export of IdCredSec (BLS12-381) | 
| `0x07` | `0x04` | `0x00` or `0x01` | `identity_provider[uint32] identity[uint32]` | Export of blinding randomness (BLS12-381) |
| `0x07` | `0x05` | `0x00` or `0x01` | `identity_provider[uint32] identity[uint32] credential_counter[uint32] attribute_tag[uint32]` | Export of attribute randomness (BLS12-381) |

* Multi command

- p2 = `0x00` or `0x01`
- idp = `identity_provider[uint32]`
- id = `identity[uint32]`

| INS | P1 | P2 | CDATA | Comment |
|-----|--------|--------|------------|----|
| `0x07` | `0x00` | p2 | idp id | Ask Permission to export any identity data. |
| `0x07` | P1 | p2 | CDATA | Export the corresponding data without prompt, check the single command description for specific P1/CDATA. This step can be repeated until the last step is executed |
| `0x07` | `0x00` | p2 | idp id | Revoke Permission to export identity data. |

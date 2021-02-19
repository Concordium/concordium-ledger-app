# Export private key seed

A utility method that exports a private key for exactly these three types:

1. ID_cred_sec
1. PRF key
1. Anonymity revoker decryption key

The exported keys will be derived using SLIP10 for ed25519, and their purpose is to serve as the key seed for the 
KeyGen algorithm for generating BLS12-381 private keys.

Note that this restriction means that it is not possible to export keys that are used for signatures. Signature keys
do not leave the device at any point, and the exported keys cannot be used to submit transactions.

## Protocol description

* Single command

| P1 | CDATA | Comment |
|--------|-------------|----|
| `0x00` or `0x01` | `identity` | `P1 = 0x00 = ID_cred_sec` <br/> `P1 = 0x01 = PRF key` |
| `0x02` | `idp ar_index` | Anonymity revoker decryption key |
# Verify Address

Given an identity index and a credential counter/account index this function displays the associated account address.

Allows the user to approve or reject the address, determining whether the app returns success or rejection.

For an account address to match the one that is displayed, the account's primary credential must be made using the PRF-key exported by the same Ledger on the same identity index and credential counter.
The address is the credId's sh256 hash, and is displayed in base58.

if P2 = 0x00, then the Legacy path is used. If P2 = 0x01, then the Mainnet path/coinType is used, if P2 = 0x02 then the Testnet path/coinType is used instead. Note that the Legacy path expects only the identity and credCounter, while the other paths also expects the identityProvider index to be given.

## Protocol description

* Single command

INS | P1 | P2 | CDATA | Comment |
|--------|--------|--------|------------|----|
| `0x00` | `0x00` | `0x00` | `identity[uint32] credCounter[uint32]` |  |
| `0x00` | `0x00` | `0x01` | `identityProvider[uint32] identity[uint32] credCounter[uint32]` |  |
| `0x00` | `0x00` | `0x02` | `identityProvider[uint32] identity[uint32] credCounter[uint32]` |  |


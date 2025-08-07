# Verify Address

Given an identity index and a credential counter/account index this function displays the associated account address.

Allows the user to approve or reject the address, determining whether the app returns success or rejection.

For an account address to match the one that is displayed, the account's primary credential must be made using the PRF-key exported by the same Ledger on the same identity index and credential counter.
The address is the credId's sh256 hash, and is displayed in base58.

## Protocol description

- Single command

| INS    | P1     | P2     | CDATA                                                             | Comment                                    |
| ------ | ------ | ------ | ----------------------------------------------------------------- | ------------------------------------------ |
| `0x00` | `0x00` | `0x00` | `identity[uint32] credCounter[uint32]`                            | Legacy derivation path                     |
| `0x00` | `0x01` | `0x00` | `identityProvider[32 bytes] identity[uint32] credCounter[uint32]` | New derivation path with identity provider |

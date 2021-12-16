# Verify Address

Given identity index and a credential counter/account index, displays the associated account address.

Allows the user to approve or reject the address, determing whether the app returns success or rejection.

## Protocol description

* Single command

INS | P1 | P2 | CDATA | Comment |
|--------|--------|--------|------------|----|
| `0x00` | `0x00` | `0x00` | `identity[uint32] credCounter[uint32]` |  |

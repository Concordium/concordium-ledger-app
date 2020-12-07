# Transfer to public

A transaction for transferring an amount to the encrypted balance of the account.

## Protocol description

* Single command.

INS | P1 | CDATA | Comment |
|---|--------|-------------|----|
| `0x11` | `0x00` | `path_length path transaction_header transaction_kind amount` | |

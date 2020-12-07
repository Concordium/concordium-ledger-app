# Transfer transaction

A transaction to transfer GTU from one account to another.

## Protocol description

* Single command

| P1 | CDATA | Comment |
|--------|-------------|----|
| `0x00` | `path_length path transaction_header transaction_kind to_address amount` | |

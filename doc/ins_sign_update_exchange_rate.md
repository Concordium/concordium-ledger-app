# Update exchange rate

This instruction supports signing two separate update transaction types. A single instruction covers both transactions as the
payload body of both transactions are identical.

1. Update euro per energy
1. Update µGTU per Euro

## Protocol description

* Single command

INS | P1 | CDATA | Comment |
|---|--------|-------------|----|
| 0x06 | `0x00` | `path_length path update_header update_type exchange_rate_numerator exchange_rate_denominator` | `update_type = 0x03 = update euro per energy` <br/> `update_type = 0x04 = update µGTU per Euro` |

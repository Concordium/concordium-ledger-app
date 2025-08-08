# Update pool parameters

Used for updating:
 - the commission rates for passive delegation
 - the allowed commission ranges that a baker can choose
 - the minimum equity capital required for a new baker
 - the maximum fraction of the total staked capital that a new baker can have (capital_bound)
 - the maximum leverage that a baker can have as a ratio of total stake to equity capital (leverage_bound)

## Protocol description

* Multiple commands

INS | P1 | P2 | CDATA | Comment |
|----|--------|-----|-------------|----|
| `0x41` | `0x00` | `0x00` | `path_length path[uint32]x[5] update_instruction_header[28 bytes] update_type[uint8] passive_delegation_commissions[uint32]x[3]` | The rates are considered as fractions out of 100000 |
| `0x41` | `0x01` | `0x00` | `commission_bounds[uint32]x[6]` | The bounds are considered as fractions out of 100000 |
| `0x41` | `0x02` | `0x00` | `minimum_equity_capital[uint64] capital_bound[uint32] leverage_bound[uint64]x[2]` |  The capital bound is the considered as a fraction out of 100000 |

## Transaction Flow

1. **Review**: Shows a summary of the transaction.
2. **Update type**: Shows the human-readable update type ("Pool parameters").
3. **Passive delegation commissions**: Shows commission rates for passive delegation.
4. **Commission bounds**: Shows the allowed commission ranges.
5. **Minimum equity capital**: Shows the minimum equity capital required.
6. **Capital bound**: Shows the maximum fraction of total staked capital.
7. **Leverage bound**: Shows the maximum leverage ratio.
8. **Sign/Decline**: User can approve or reject the transaction.

# Update finalization committee parameters

Used for updating the finalization committee parameters. This controls how many finalizers there are, and how much relative stake is required to be part of the finalization committee.

## Protocol description

* Single command

INS | P1 | P2 | CDATA | Comment |
|----|--------|-----|-------------|----|
| `0x46` | `0x00` | `0x00` | `path_length path[uint32]x[5] update_instruction_header[28 bytes] update_type[uint8] min_finalizers[uint32] max_finalizers[uint32] relative_stake_threshold_fraction[uint32]` | Update type must be 22. The relative stake threshold fraction is considered as a fraction out of 100000. |

## Transaction Flow

1. **Review**: Shows a summary of the transaction.
2. **Update type**: Shows the human-readable update type ("Finalization committee parameters").
3. **Min finalizers**: Shows the minimum number of finalizers.
4. **Max finalizers**: Shows the maximum number of finalizers.
5. **Stake threshold**: Shows the relative stake threshold.
6. **Sign/Decline**: User can approve or reject the transaction.

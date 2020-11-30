# Update authorizations

A transaction for updating authorizations, i.e. which keys can be used to update different types of parameters on the 
chain.

## Protocol description

* Multiple commands

INS | P1 | CDATA | Comment |
|---|--------|-------------|----|
| 0x08 | `0x00` | `path_length path update_header update_type` | |
| 0x08 | `0x01` | `public_key_list_length` | |
| 0x08 | `0x02` | `public_key` | One key per message, repeated until all keys have been sent. |
| 0x08 | `0x03` | `access_structure_key_indices_length` | Contains the number of key indices for the current access structure. |
| 0x08 | `0x04` | `access_structure_key_indicies_list` | Contains the key indices for the current access structure. Can be split over multiple commands if there are more than 127. |
| 0x08 | `0x05` | `access_structure_threshold` | Contains the key threshold for the current access structure |

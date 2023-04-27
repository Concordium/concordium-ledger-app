# Update contract

A transaction to update a contract instance.
The payload contains the following fields:

- amount: microCCD sent to the contract instance. 
- index: contract instance index.
- subindex: contract instance subindex.
- receive name: `\<contractName\>.\<functionName\>`.
- parameter: Specific message for the contract / the paramaters for the function.

The length of the receive name and parameter is sent together with the remaining field values in the initial batch.
Note that that the receive name is required to have a length at least 1, and if parameters length is 0, the signature is returned on the last receiveName batch (i.e. P1 = 2 is skipped).

## Protocol description

* Multiple commands

INS | P1 | P2 | CDATA | Comment |
|----|--------|-----|-------------|----|
| `0x36` | `0x00` | `0x00` | `path_length path[uint32]x[8] account_transaction_header[60 bytes] transaction_kind[uint8] amount[uint64] index[uint64] subindex[uint64] receiveName length[uint16] parameter length[uint16]` |  |
| `0x36` | `0x01` | `0x00` | `receiveName[1..255 bytes]` | In batches of up to 255 bytes until the entire receive name has been sent |
| `0x36` | `0x02` | `0x00` | `parameter[1..255 bytes]` | In batches of up to 255 bytes until the entire parameter has been sent |

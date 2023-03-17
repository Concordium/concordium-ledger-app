# Init contract

A transaction to initalize a contract instance.
The payload contains the following fields:

- amount: microCCD sent to the contract instance. 
- module reference: module reference for the module containing for the contract source.
- contract name: string that identifies the contract.
- parameter: Specific message for the contract / the parameters for the init function.

The length of the contract name and parameter is sent together with the remaining field values in the initial batch.
Note that that the contract name is required to have a length at least 1, and if parameters length is 0, the signature is returned on the last contract name batch (i.e. P1 = 2 is skipped).

## Protocol description

* Multiple commands

INS | P1 | P2 | CDATA | Comment |
|----|--------|-----|-------------|----|
| `0x37` | `0x00` | `0x00` | `path_length path[uint32]x[8] account_transaction_header[60 bytes] transaction_kind[uint8] amount[uint64] module_reference[32 bytes] contract_name_length[uint16] parameter_length[uint16]` |  |
| `0x37` | `0x01` | `0x00` | `contract_name[1..255 bytes]`  | In batches of up to 255 bytes until the entire contract name has been sent |
| `0x37` | `0x02` | `0x00` | `parameter[1..255 bytes]` | In batches of up to 255 bytes until the entire parameter has been sent |

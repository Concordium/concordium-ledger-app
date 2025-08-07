# Configure baker

A transaction to configure a baker.

All parameters of the payload are optional, and the first part of the payload is a bitmap used to indicate which of the optional parameters that are sent. 8 bits are used:

- index 0: capital amount
- index 1: restake earnings
- index 2: open for delegation
- index 3: Baker keys
- index 4: metadata url
- index 5: transaction fee commission
- index 6: baking reward commission
- index 7: finalization reward commission

The Ledger expects that only necessary parts are sent. i.e. if the 4th bit in the bitmap is not set, there should not be a metadata url, and so the P1 = 2 and P1 = 3 parts should be skipped.

## Protocol description

- Multiple commands

| INS    | P1     | P2     | CDATA                                                                                                                                                                                                                                                                              | Comment                                                                                                                                               |
| ------ | ------ | ------ | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------- |
| `0x18` | `0x00` | `0x00` | `path_length path[uint32]x[8] account_transaction_header[60 bytes] transaction_kind[uint8] bitmap[uint16] capitalAmount[uint64] restakeEarnings[uint8] openForDelegation[uint8] electionVerifyKey[uint32] electionProof[uint64] signatureVerifyKey[uint32] signatureProof[uint64]` | restakeEarnings should be 0 or 1, openForDelegation should be 0,1 or 2 (representing "open for all", "Closed for new", "Closed for all" respectively) |
| `0x18` | `0x01` | `0x00` | `aggregationVerifyKey[uint96] aggregationProof[uint64]`                                                                                                                                                                                                                            |                                                                                                                                                       |
| `0x18` | `0x02` | `0x00` | `metadataUrlLength[uint16]`                                                                                                                                                                                                                                                        |                                                                                                                                                       |
| `0x18` | `0x03` | `0x00` | `metadataUrl[1..255 bytes]`                                                                                                                                                                                                                                                        | In batches of up to 255 bytes, repeated until the entire URL has been sent.                                                                           |
| `0x18` | `0x04` | `0x00` | `transactionFeeCommissionRate[uint32] bakingRewardCommissionRate[uint32] finalizationRewardCommissionRate[uint32]`                                                                                                                                                                 |                                                                                                                                                       |

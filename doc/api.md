# Concordium Ledger Nano S API

This document contains the description of the API for the Concordium application. It should be used when implementing
the software that integrates with the application.

## Communication protocol

The Ledger device uses APDU (Application Protocol Data Unit) messages for the communication between a computer host and 
the Ledger device. A very brief introduction to the protocol can be 
found [here](https://en.wikipedia.org/wiki/Smart_card_application_protocol_data_unit), but it will help to provide
an overview of the Concordium specific messages below.

## Signing simple transfer

### Description

This command returns a signature for the sha256 hash of a simple account transfer transaction. On the device
the recipient address is displayed, and the amount to transfer before accepting to sign the transaction hash. After signing the
signature is shown on the device so that it can be compared with the value shown on the computer. The user should stop 
any use of the computer application if there is a mismatch. 

### Coding

The input APDU message:

| CLA | INS | P1 | P2 | Lc                       | Command data   | Le                            | 
|-----|-----|----|----|--------------------------|----------------|-------------------------------|
| E0  | 02  | Account index (1 byte, 0-255) | 00 | length(transaction header + transaction payload) | Transaction header + transaction body | Variable (not set explicitly) |

A successful output has the following format, where the status word '9000' indicates a success. The signature bytes
is the signature on the hash of the transaction.

| Signature bytes                     | Status word |
|-------------------------------------|-------------|
| 128 bytes of hexadecimal characters | 9000        |

## Get public-key for an account

### Description

This command returns a public-key for the given account number. On the device the user has to accept that a public-key
is going to be derived and returned to the computer. Afterwards the user will be shown the public-key, so that a 
comparison between the public-key on the computer and the device can be performed. The user should stop any use
of the computer application if there is a mismatch.

### Coding

The input APDU message:

| CLA | INS | P1 | P2 | Lc                       | Command data   | Le                            | 
|-----|-----|----|----|--------------------------|----------------|-------------------------------|
| E0  | 01  | 00 | 00 | Length of account number | Account number | Variable (not set explicitly) |

Here 'Account number' should contain the account number to retrieve a public-key for, using up at most 4 bytes (uint32_t).
A successful output has the following format, where the status word '9000' indicates a success.

| Public-key bytes                   | Status word |
|------------------------------------|-------------|
| 64 bytes of hexadecimal characters | 9000        |
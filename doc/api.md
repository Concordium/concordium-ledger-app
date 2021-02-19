# Concordium Ledger Nano S API

This document contains the description of the API for the Concordium application. It should be used when implementing
the software that integrates with the application.

## Communication protocol

The Ledger device uses APDU (Application Protocol Data Unit) messages for the communication between a computer host and 
the Ledger device. A very brief introduction to the protocol can be 
found [here](https://en.wikipedia.org/wiki/Smart_card_application_protocol_data_unit), but it will help to provide
an overview of the Concordium specific messages below.

For the specific protocol for a specific function, please see the protocol documents prefixed with 'ins'.

## Key derivation path

For all instructions a key derivation path has to be provided. This is always sent as the initial bytes in the first
command. So any first command for a function on the Ledger starts with:

```CLA INS P1 P2 Lc path_length path```

It is necessary to include the path length as it is not static, i.e. some paths are longer than others in our 
setup.

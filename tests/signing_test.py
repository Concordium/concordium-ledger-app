# This file contains a small test using ledgerblue for quick interfacing with the Ledger device.
# It demonstrates the APDU message that has to be sent to the running Concordium app on the Ledger device, for
# it to start the signing of a simple transfer transaction.
from ledgerblue.comm import getDongle
import argparse

parser = argparse.ArgumentParser()
parser.add_argument('--account', help="The BIP32 account to retrieve a public-key for, e.g. \"34\".")
args = parser.parse_args()

if args.account == None:
    raise Exception("Please provide an account. Use --help for details.")

# We also need to get the account address as input... So not just     getPrivateKeyBasic(keyPath, 5, privateKey);the transaction header in the initial
# exchange of data. Or perhaps split it into two different initial exchanges, because not all transactions will
# have a transaction header.
account = '{:02x}'.format(int(args.account))
print(account)

# Build APDU message that prompts the Concordium Ledger application to start the signing-flow.
# CLA 0xE0 (has to be set to this always, if not the message will be rejected)
# INS 0x02 (instruction code for signing a transaction)
# P1  0x00 (unused)
# P2  0x00 (unused)

# Key derivation path definition (account subtree, signature account usage, account index 0)
path = "00" + account

# Transaction header
address = "2ae13c414a03176e887c91b7f93f47af4c1130c5ece7b9e250113ea40beb0e4d"
nonce = "0000000000000083"
energy = "0000000000001388"
payload_size = "00000000"
expiry = "0000017579871035"
transaction_header = address + nonce + energy + payload_size + expiry

# Transaction payload for a simple transaction
transaction_kind = "03"
to_address = "2ae13c414a03176e887c91b7f93f47af4c1130c5ece7b9e250113ea40beb0e4d"
amount = "00000000003000FF"
transaction_payload = transaction_kind + to_address + amount

# Note the size here is incorrect
message = "E0020000" + '{:02x}'.format(60) + path + transaction_header + transaction_payload
print(message)
apdu = bytearray.fromhex(message)

print("Requesting for a signature for a transaction using account: " + args.account)

dongle = getDongle(True)
result = dongle.exchange(apdu)

print(result.hex())
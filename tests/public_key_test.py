# This file contains a small test using ledgerblue for quick interfacing with the Ledger device.
# It demonstrates the APDU message that has to be sent to the running Concordium app on the Ledger device, for
# it to start the public-key flow.
from ledgerblue.comm import getDongle
import argparse

parser = argparse.ArgumentParser()
parser.add_argument('--account', help="The BIP32 account to retrieve a public-key for, e.g. \"315\".")
args = parser.parse_args()

if args.account == None:
    raise Exception("Please provide an account. Use --help for details.")

account = '{:08x}'.format(int(args.account))

# Build APDU message that prompts the Concordium Ledger application to start the public-key retrieval flow.0x00
# CLA 0xE0 (has to be set to this always, if not the message will be rejected)
# INS 0x01 (instruction code for getting the public-key)
# P1  0x00 (unused)
# P2  0x00 (unused)
# Total message becomes:
# E0010000 + lengthOfAccount + account
message = "E0010000" + '{:02x}'.format(len(account) + 1) + account
apdu = bytearray.fromhex(message)

print("Requesting public-key for account: " + args.account)

dongle = getDongle(True)
result = dongle.exchange(apdu)

print(result.decode("utf-8"))
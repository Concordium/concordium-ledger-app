# Changelog

## 2.1.0 - TBD

### Added

* Added flow to verify account address.

## 2.0.3 - 2021-11-29

### Changed

* Removed references to GTU in the UI.

## 2.0.2 - 2021-10-21

### Added
* An acceptance step has been added to the export of private key seeds.

## 2.0.1

* Fixed an issue in the add baker UI, where a response could be sent before signing or declining.

## 2.0.0

* Improved state validation to deny instruction changes in multi command transactions.
* Support building for the Ledger Nano X.
* Simplified the UI by updating terminology and stopped displaying details that cannot feasibly be verified by a user.
* Export of private key seeds has been changed so that either the PRF key can be exported alone, or the PRF key and the IdCredSec are exported in a single command.
* Added support for transactions with memos.
* Support for the "Add identity provider" update.
* Support for the "Add anonymity revoker" update.
* Improved pagination of account addresses and hexadecimal strings, so that pages are split evenly and consistently.

## 1.0.2

* Scheduled transfer release time now shows October as '10' instead of '010'.

## 1.0.1

* Scheduled transfers now display the release time as a human readable date time string.
* Fixed UI bug in remove baker transaction.

## 1.0.0

Release candidate.

* Stopped showing the hash of attributes for credential deployments.

## 0.6.2

* Fixed bug in credential deployment where the application version was shown as being the sender, when no sender should be shown.

## 0.6.1

* Fixed an issue with parsing of GTU amounts that could break on certain amounts.
* Fixed bug in hashing of authorization keys update that made the signature invalid.

## 0.6.0

* Sender account address is now shown for all account transactions.
* Improved the UI for encrypted transfer and transfer to public account transactions.
* Scheduled transfer 'Timestamp' title renamed to 'Release time'.

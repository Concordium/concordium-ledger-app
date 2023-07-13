# Changelog

## 4.1.2

### Changed

* Removed usage of a number of deprecated functions supplied by Ledger.

## 4.1.1

### Changed

* Improved the UI of the credential deployment transaction to be more seamless.

## 4.1.0

### Added

* Bluetooth and airplane mode handling for Nano X. Experimental.

### Changed

* Updated application to run Ledger's guidelines enforcer.

## 4.0.1

### Changed

* Transfer amounts are now prefixed with CCD inline with the amount, instead of (CCD) in the upper line.

## 4.0.0

### Changed

* Transfer amounts now show (CCD) in upper line.
* Compare now has accept/reject screens.
* signIpInfo initial screen be part of the 1st public key flow.
* Improved the UI flow for transactions that can contain a memo to make it more seamless, i.e. require fewer "both button clicks".

### Removed

* Support for chain updates
* Support for old baker transactions

## 3.1.0

### Added

* Support for the Nano S Plus 1.1.0 firmware.

## 3.0.1

### Added

* Support for Ledger Nano S Plus.

### Changed

* The commission rates UI is now prefixed with a screen that says 'Commission rates'.
* When setting stake amount to 0 the UI now shows 'Stop baking' instead of the 0 value.
* When setting delegation amount to 0 the UI now shows 'Stop delegation' instead of the 0 value.
* Static fractions (numerator/100000), e.g. commission rates for bakers, are now shown as percentages.

## 3.0.0

### Added

* Added flow to verify account address.
* Added support for register data transaction.
* Support for the configure delegation transaction.
* Support for the configure baker transaction.
* Support for version 1 of the "Authorizations" update.
* Support for version 1 of the "Mint distribution" update.
* Support for the "Time parameter" update.
* Support for the "Pool parameter" update.
* Support for the "Cooldown parameter" update.

### Changed

* References to GTU in exchange rate update and authorization update changed to CCD.

### Removed

* Support for version 0 of the "Authorizations" update.
* Support for version 0 of the "Mint distribution" update.

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

# Changelog

## 1.1.0 (Work in progress)

* Improved state validation to deny instruction changes in multi command transactions.
* Support building for the Ledger Nano X.
* Simplified the UI by updating terminology and stopped displaying details that cannot feasibly be verified by a user.
* Changed Hex and Base58Check functions to ensure splitting pages evenly and consistently.

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

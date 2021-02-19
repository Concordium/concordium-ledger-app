# ledger-app

An application for the Ledger Nano S for retrieving public-keys and signing data.

## Deploying application to Ledger Nano S

Start by following the [official guide](https://ledger.readthedocs.io/en/latest/userspace/getting_started.html) to 
set your environment up correctly with the required dependencies. The currently used SDK is the up-to-date [master 
branch](https://github.com/LedgerHQ/nanos-secure-sdk).

The Makefile is responsible for loading the application onto the device. This is done with the load
target, while the device is connected via USB.

```
cd ledger-app/
make load
```

To delete the application from the device the same Makefile is used, but by using the delete target.

```
cd ledger-app/
make delete
```

Both scripts require you to respond to the installation UI on the device for the installation or deletion
to complete.

## Developing for the Ledger

Refer to the official documentation provided by Ledger. For quick development when deploying to the 
device, make sure to deploy a custom certificate to the device. See the "PIN Bypass" section 
[here](https://ledger.readthedocs.io/en/latest/userspace/debugging.html).
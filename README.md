# Concordium Ledger application

An application for the Ledger Nano S, Ledger Nano S Plus and Ledger Nano X devices for retrieving keys and signing transactions.

## Secure SDK dependencies

We depend on [Nano S Secure SDK](https://github.com/LedgerHQ/nanos-secure-sdk/) and [Ledger Secure SDK](https://github.com/LedgerHQ/ledger-secure-sdk/), which have been added as git submodules. Make sure to initialize submodules when checking out this repository:
```
git submodule update --init
```

## Building and deploying
We provide a small Dockerfile that wraps [ledger-app-builder](https://github.com/LedgerHQ/ledger-app-builder) which can be used for building, loading and deleting an application for the Ledger Nano S and Ledger Nano S Plus devices. Our Dockerfile provides the dependency required for zipping a release for sideloading.

To build the Docker image run:
```bash
docker build -t concordium/ledger-app-builder .
```
You can now run the Docker container with
```bash
docker run --rm -ti -v "/dev/bus/usb:/dev/bus/usb" -v "$(realpath .):/app" --privileged concordium/ledger-app-builder:latest
```
You now have access to the commands provided by the Makefile:

```sh
# Load the application onto the connected device
make load # this does not work for macos/windows out of the box, due to limited usb support.
```

```sh
# Delete the application from the connected device
make delete
```

```sh
# Switch BOLOS_SDK to build for Nano S Plus
# Note that 'make clean' is a requirement when switching BOLOS_SDK.
export BOLOS_SDK=$NANOSP_SDK
make clean
make
```

### Loading onto the ledger (alternative to make load)

This approach requires `python`, `pip` the pip module `ledgerblue` to be installed locally on the machine.

```
python -m ledgerblue.loadApp --targetId <target-id> --apiLevel 24 --fileName bin/app.hex --appName <app-name> --appVersion <app-version> --delete --tlv
```

- **app-name**: the application name used on the device.
  - "Concordium" for the the app used for regular users
  - "CCDGovernance" for the governance app
- **app-version**: the version of the application.
- **target-id**: the target id of the device.
  - "0x33100004" for nano S+

### For the Speculos emulator

As the Ledger Nano X does not support sideloading, the only way to test updates on a Nano X is 
to use the [Speculos emulator](https://github.com/LedgerHQ/speculos). Please follow their documentation
for how to setup the emulator. To build the `.elf` file required by the emulator run:
```
export BOLOS_SDK=$NANOX_SDK
make emulator
```
The file will be available at `bin/app.elf`.

## Developing for the Ledger

Refer to the official documentation provided by Ledger. For quick development when deploying to the 
device, make sure to deploy a custom certificate to the device. See the "PIN Bypass" section 
[here](https://developers.ledger.com/docs/nano-app/debug/).

For documentation of the exposed functionality and how to integrate with the Concordium specific 
applications, please take a look [here](doc/api.md).

## Linting
A make target is available for linting:
```bash
make lint
```

## Testing

### Running unit tests
There are unit tests on some of the functions that do not rely on Ledger specific libraries.
First, you must have the following installed:

- CMake >= 3.10
- CMocka >= 1.1.5

To build the tests:
```bash
cd unit_tests
cmake -Bbuild -H. && make -C build
```
While still in the `unit_tests` directory, execute the following to run the unit tests:
```bash
CTEST_OUTPUT_ON_FAILURE=1 make -C build test
```

### Running end to end tests
An end to end test is available for each instruction implemented in the application. The end
to end tests depend on having built the application for Nano S, Nano SP and Nano X, and having placed
their `.elf` files correctly. This can achieved by running (from within Docker):
```bash
cd tests
./build_binaries.sh
```
To fetch the required dependencies run:
```bash
yarn
```

While still in the `tests` directory, execute the following to run the end to end tests:
```bash
yarn test
```

## Building a release
Note that it is only possible to build a release for the Ledger Nano S and the Ledger Nano S plus. This is because only those devices allow for sideloading of an application.

To make a new release of the Concordium Ledger application you must use the Docker setup described above.

Additionally you must set the following environment variables
```
LEDGER_SIGNING_KEY=private_key_used_for_signing_releases
LEDGER_PUBLIC_KEY=public_key_matching_the_signing_key
```
To build a new release make sure that `APPVERSION` has been bumped correctly, and then run
```
make clean
export BOLOS_SDK=$NANOS_SDK
make release

make clean
export BOLOS_SDK=$NANOSP_SDK
make release
```
The release will be packaged into two `.zip` archives, each with the required binary and the corresponding install scripts.

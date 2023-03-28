# Concordium Ledger application

An application for the Ledger Nano S, Ledger Nano S Plus and Ledger Nano X devices for retrieving keys and signing transactions.

## Secure SDK dependencies

We depend on [Nano S Secure SDK](https://github.com/LedgerHQ/nanos-secure-sdk/), [Nano S Plus Secure SDK](https://github.com/LedgerHQ/nanos-secure-sdk/) and the [Nano X Secure SDK](https://github.com/LedgerHQ/nanosplus-secure-sdk), which 
have been added as git submodules. Make sure to initialize submodules when checking out this repository:
```
git submodule update --init
```

## Building and deploying

### The Docker way
We provide a small Dockerfile that wraps [ledger-app-builder](https://github.com/LedgerHQ/ledger-app-builder) which can be used for building, loading and deleting an application for the Ledger Nano S and Ledger Nano S Plus devices. Our Dockerfile ensures that we can control the secure SDK dependencies, as they are hardcoded in Ledger's image, and provides the dependency required for zipping a release for sideloading.

To build the Docker image run:
```bash
docker build -t concordium/ledger-app-builder .
```
You can now run the Docker container with
```bash
docker run --rm -ti -v "/dev/bus/usb:/dev/bus/usb" -v "$(realpath .):/app" --privileged concordium/ledger-app-builder:latest
```
You now have access to the commands provided by the Makefile:
```
// Load the application onto the connected device
root@410e7ab19bea:/app# make load

// Delete the application from the connected device
root@410e7ab19bea:/app# make delete

// Switch BOLOS_SDK to build for Nano S Plus
// Note that 'make clean' is a requirement when switching BOLOS_SDK.
root@f382ee774923:/app# export BOLOS_SDK=nanosplus-secure-sdk
root@f382ee774923:/app# make clean
root@f382ee774923:/app# make load
```

### Without Docker
It is possible to setup a local Ubuntu environment to build and deploy Ledger applications without the user of Docker. The best place to check the required tools for building is in the [ledger-app-builder Dockerfile](https://github.com/LedgerHQ/ledger-app-builder/blob/master/Dockerfile). We do not provide a full guide to install these as the Docker image can be used as an alternative. Note that there is some maintenance required as the dependencies used may change over time, so on updates to SDK's your setup may break.

When your dependencies are setup you can load the application with
```bash
make load
```
and delete the application from the device with
```bash
make delete
```
Switching `BOLOS_SDK` to build for a different device:
```bash
export BOLOS_SDK=nanosplus-secure-sdk
make clean
make load
```

### For the Speculos emulator

As the Ledger Nano X does not support sideloading, the only way to test updates on a Nano X is 
to use the [Speculos emulator](https://github.com/LedgerHQ/speculos). Please follow their documentation
for how to setup the emulator. To build the `.elf` file required by the emulator run:
```
export BOLOS_SDK=nanox-secure-sdk (or nanos-secure-sdk/nanosplus-secure-sdk)
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
to end tests depend on having built the application for Nano S and Nano X, and having placed
their `.elf` files correctly. This can achieved by running:
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

To make a new release of the Concordium Ledger application you must have set up the build
environment like described above (either the Docker setup or a local setup).

Additionally you must set the following environment variables
```
LEDGER_SIGNING_KEY=private_key_used_for_signing_releases
LEDGER_PUBLIC_KEY=public_key_matching_the_signing_key
```
To build a new release make sure that `APPVERSION` has been bumped correctly, and then run
```
make clean
export BOLOS_SDK=nanos-secure-sdk
make release

make clean
export BOLOS_SDK=nanosplus-secure-sdk
make release
```
The release will be packaged into two `.zip` archives, each with the required binary and the corresponding install scripts.

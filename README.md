# Concordium Ledger application

An application for the Ledger Nano S and Ledger Nano X devices for retrieving keys and signing transactions.

## Secure SDK dependency

We depend on the [Nano S Secure SDK](https://github.com/LedgerHQ/nanos-secure-sdk/) and the 
[Nano X Secure SDK](https://github.com/LedgerHQ/nanox-secure-sdk), which 
have been added as git submodules. Make sure to initialize submodules when checking out this repository:
```
git submodule update --init
```

## Building and deploying application to Ledger Nano S
Ledger has documentation on [how to build an application](https://developers.ledger.com/docs/nano-app/build/) and [load it onto a Nano S](https://developers.ledger.com/docs/nano-app/load/), using a docker image. 
If you try to setup your local environment to build, note that it is important to use the correct version of `clang` for the build to work (currently 9.0.0).
If your version is incompatible, then it is quite likely that you will see an error stating that
`ld.lld doesn't exist`.

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

### Building using the provided Dockerfile

First, build the docker image

```bash
docker build -t concordium/ledger-app-builder .
```
Then, run the docker container

```bash
docker run --rm -ti -v "$(realpath .):/app" --privileged concordium/ledger-app-builder
```

This launches bash inside ubuntu. The app can now be built using the commands from the Makefile. 
It still seems like it's only possible to load the application onto the ledger with `make load` on an Ubuntu machine, due to docker not having access to usb.

### Switching the SDK

Please note that it is necessary to run
```
make clean
```
when switching the SDK used to build the application.

## Developing for the Ledger

Refer to the official documentation provided by Ledger. For quick development when deploying to the 
device, make sure to deploy a custom certificate to the device. See the "PIN Bypass" section 
[here](https://developers.ledger.com/docs/nano-app/debug/).

For documentation of the exposed functionality and how to integrate with the Concordium specific 
applications, please take a look [here](doc/api.md).

## Building for the Speculos emulator

As the Ledger Nano X does not support sideloading, the only way to test updates on a Nano X is 
to use the [Speculos emulator](https://github.com/LedgerHQ/speculos). Please follow their documentation
for how to setup the emulator. To build the `.elf` file required by the emulator run:
```
export BOLOS_SDK=nanox-secure-sdk (or nanos-secure-sdk for a Nano S device)
make emulator
```
The file will be available at `bin/app.elf`.

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
to end tests depends on having built the application for Nano S and Nano X, and having placed
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

Note that it is only possible to build a release for the Nano S.

To make a new release of the Concordium Ledger application you must have set up the build
environment like described in the [official guide](https://developers.ledger.com/docs/nano-app/build/).
Additionally you must set the following environment variables
```
LEDGER_SIGNING_KEY=private_key_used_for_signing_releases
LEDGER_PUBLIC_KEY=public_key_matching_the_signing_key
```
To build a new release make sure that `APPVERSION` has been bumped correctly, and then run
```
export BOLOS_SDK=nanos-secure-sdk
make release
```
The release will be packaged into a `.zip` archive with the required binary and the corresponding install scripts.

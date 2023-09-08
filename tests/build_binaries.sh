mkdir -p bin/nanos
mkdir -p bin/nanox
mkdir -p bin/nanosplus
mkdir -p bin/governance-nanos

cd ..

BOLOS_SDK=$NANOS_SDK make clean
BOLOS_SDK=$NANOS_SDK make

cp bin/app.elf "tests/bin/nanos/concordium_nanos.elf"

BOLOS_SDK=$NANOSP_SDK make clean
BOLOS_SDK=$NANOSP_SDK make

cp bin/app.elf "tests/bin/nanosplus/concordium_nanosplus.elf"

BOLOS_SDK=$NANOX_SDK make clean
BOLOS_SDK=$NANOX_SDK make

cp bin/app.elf "tests/bin/nanox/concordium_nanox.elf"

# Build governance app

cd governance-app

BOLOS_SDK=$NANOS_SDK make clean
BOLOS_SDK=$NANOS_SDK make

cp bin/app.elf "../tests/bin/governance-nanos/ccdGovernance_nanos.elf"

BOLOS_SDK=$NANOSP_SDK make clean
BOLOS_SDK=$NANOSP_SDK make

cp bin/app.elf "../tests/bin/governance-nanosplus/ccdGovernance_nanosplus.elf"

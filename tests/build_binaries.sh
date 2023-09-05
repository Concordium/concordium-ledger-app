mkdir -p bin/nanos
mkdir -p bin/nanox
mkdir -p bin/nanosplus
mkdir -p bin/governance/nanos
mkdir -p bin/governance/nanox

cd ..

cd governance-app

BOLOS_SDK=$NANOS_SDK make clean
BOLOS_SDK=$NANOS_SDK make

cp bin/app.elf "../tests/bin/governance/nanos/ccdGovernance_nanos.elf"

BOLOS_SDK=$NANOX_SDK make clean
BOLOS_SDK=$NANOX_SDK make

cp bin/app.elf "../tests/bin/governance/nanox/ccdGovernance_nanox.elf"

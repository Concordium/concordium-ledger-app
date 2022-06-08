mkdir -p bin/nanos
mkdir -p bin/nanox
mkdir -p bin/nanosplus

cd ..

BOLOS_SDK=nanos-secure-sdk make clean
BOLOS_SDK=nanos-secure-sdk make

cp bin/app.elf "tests/bin/nanos/concordium_nanos.elf"

BOLOS_SDK=nanosplus-secure-sdk make clean
BOLOS_SDK=nanosplus-secure-sdk make

cp bin/app.elf "tests/bin/nanosplus/concordium_nanosplus.elf"

BOLOS_SDK=nanox-secure-sdk make clean
BOLOS_SDK=nanox-secure-sdk make

cp bin/app.elf "tests/bin/nanox/concordium_nanox.elf"

mkdir -p bin/nanos
mkdir -p bin/nanox
mkdir -p bin/nanosplus

cd ..

TARGET=nanos make clean
TARGET=nanos make

cp bin/app.elf "tests/bin/nanos/concordium_nanos.elf"

TARGET=nanos2 make clean
TARGET=nanos2 make

cp bin/app.elf "tests/bin/nanosplus/concordium_nanosplus.elf"

TARGET=nanox make clean
TARGET=nanox make

cp bin/app.elf "tests/bin/nanox/concordium_nanox.elf"

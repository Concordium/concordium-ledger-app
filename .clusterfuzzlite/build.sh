#!/bin/bash -eu

# build fuzzers

pushd fuzzing
cmake -DBOLOS_SDK=../BOLOS_SDK -Bbuild -H.
make -C build
mv ./build/standalone_export_pk_new_path_fuzzer "${OUT}"
mv ./build/standalone_plt_fuzzer "${OUT}"
popd

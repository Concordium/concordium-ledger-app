This application is built using the 'nanos-secure-sdk' (https://github.com/LedgerHQ/nanos-secure-sdk) 
which is licensed under the license included as 'LICENSE-2.0.txt'.

Modified files used to build this release:

    - The main file for this application is a heavy modification of a main.c
      file provided by Ledger under the license included as 'LICENSE-2.0.txt'.

    - The makefile used to build this release is a modification of a makefile
      provided by Ledger under the license included as 'LICENSE-2.0.txt'.

    - The epoch seconds to date time string code used is from the musl library 
      found at https://git.musl-libc.org/cgit/musl/, with only small type and 
      naming differences. It is used under the license included as musl-MIT.txt.

    - In the testing code, we have an implementation of sha256, in sha256.c and sha256.h,
      which has been lifted from https://github.com/B-Con/crypto-algorithms, provided by
      Brad Conte (brad@bradconte.com). The code is available as part of the public domain. 

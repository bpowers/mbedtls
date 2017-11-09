# mbedtls Mac 68k
A port of mbedtls for 68k classic Macs. Designed to be built with the [Retro68 GCC cross-compiler](https://github.com/autc04/Retro68).

Allows a network-enabled 68k Mac to communicate with modern web endpoints over TLS 1.2. [Original documentation for mbedtls is here](https://github.com/ARMmbed/mbedtls).

From my limited testing so far, a 33mhz 68040 processor will complete a TLS handshake in about 15 seconds. Whereas a 16mhz 68030 will
take about 70 seconds - during which time the remote server will timeout. So it looks like the minimum requirement is a 68040.

*WARNING:* Although this library allows communication over TLS 1.2 - it should be treated as *NOT SECURE*. So far, no Entropy sources
have been written for the library which is a serious security issue. Also, classic Macs don't have a Root Certificate library which 
means you can't verify remote certificates.

Both of these can be fixed with a bit more work though...

## Building & Installing
mbedtls-Mac-68k requires Retro68 for compilation, and also the [MacTCPHelper Library](https://github.com/antscode/MacTCPHelper).

First build and install the MacTCPHelper library, then execute these commands from the top level of the mbedtls-Mac-68k directory:

    cd ..
    mkdir mbedtls-Mac-68k-build
    cd mbedtls-Mac-68k-build
    cmake ../mbedtls-Mac-68k -DCMAKE_TOOLCHAIN_FILE=<<YOUR_PATH_TO_Retro68-build>>/toolchain/m68k-apple-macos/cmake/retro68.toolchain.cmake
    make install

This will compile the library and install the library and headers into the m68k-apple-macos toolchain.
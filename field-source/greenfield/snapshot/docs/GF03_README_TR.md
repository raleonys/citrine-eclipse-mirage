# GF03 — Original HF19 Asio production ownership sources

**NOT_RELEASE, SOURCE_LINKED_REAL_TCP_TEST_ONLY.** GF02 exact parent is immutable.

GF03 carries direct identical copies of 23 source/header/test files from official HF19, plus exact unchanged bundled Asio header distribution (license included). Three original HF19 Asio real localhost TCP tests validate listener, session, service ownership using a *test stub* for LSX protocol classification/dispatch. GCC + Clang ASan/UBSan each 13/13 CTest.

Portable: `cmake -S . -B _local_gcc -G Ninja -DGREENFIELD_PORTABLE_QA_ONLY=ON -DCMAKE_CXX_COMPILER=g++`, `cmake --build _local_gcc`, `ctest --test-dir _local_gcc --output-on-failure`. Windows x64 VS Developer Prompt: `scripts\RUN_WINDOWS_GF03.cmd Release` or `Debug`. Scripts do not publish.

**NOT IMPLEMENTED:** runtime deployment, `SURGE`→service start, LSX native dispatcher, managed `DllShutdown` Asio quiescence, native Windows IOCP verification, CollegeFB27/Madden27. The original protocol stub prints `HELLO`; it is not a live LSX greeting and must never be put into a shipping DLL.

**PROVENANCE:** metadata/GF03_REUSE_LEDGER.json, metadata/manifest.json. Original HF19 license: licenses/HF19_LICENSE. Asio upstream license: licenses/ASIO_BOOST_LICENSE_1_0.txt. No new socket-owner implementation; the original HF19 Asio owner is single socket authority.

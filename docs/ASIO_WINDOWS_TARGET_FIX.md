# Asio Windows target definitions

The Production log contained the messages
`Please define _WIN32_WINNT or _WIN32_WINDOWS` and
`Assuming _WIN32_WINNT=0x0601`. These are Asio compile-target messages emitted
through MSVC `#pragma message`, not build-stopping errors.

The supplied Brownfield `CMakeLists.txt:313` explicitly defines both values:

```cmake
_WIN32_WINNT=0x0601 WINVER=0x0601
```

These definitions were missing from the Yellowfield adaptation, so Asio emitted
its default-assumption message for each affected translation unit. Both values
were added to the Windows `citrine_eclipse_mirage_asio INTERFACE` target. Application,
host, and test targets that consume it now receive the same definitions before
including the corresponding headers.

No warning-suppression flag or vendor patch was added. The previous Asio
fallback value is unchanged. These definitions select Windows API header
visibility; they do not change the Golden PE operating-system/subsystem 6.0
metadata target. The `0x0602` value observed in Bluefield is not the Brownfield
reference for this change. Other Brownfield compile settings were not copied
automatically as part of this narrowly scoped correction.

The owner confirmed that the game started with the previous package, and that
observation is retained. The Wine/cross-build and source QA results for the
current configuration are in
`metadata/evidence/ASIO_WINDOWS_TARGET_FIX.json`. An actual MSVC Production
run of the revised source was not performed in that environment.

This correction was delivered alongside the Windows IO/Gulrak removal:
[WINDOWS_IO_FIX.md](WINDOWS_IO_FIX.md).

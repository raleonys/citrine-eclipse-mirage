# Application and development exports

`preloader_l.def` is the complete application export table, with fixed ordinals:
DllMain1, SURGE2, DllShutdown3, DllWaitReady4,
and DllServerPort5. DllMain is loader-only.

`preloader.unsigned.dll` exports no C++ source APIs. Those APIs are available from
the separate `preloader.source.dll` only when PRELOADER_L_BUILD_SOURCE_API_DLL
is enabled (default: BUILD_TESTING). They require a matching compiler/CRT and are
not a shipping ABI. See [the C interface contract](../docs/PUBLIC_DLL_ABI.md).

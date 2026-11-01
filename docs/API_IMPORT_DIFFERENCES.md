> Current copy helper: [RE_COPY_RECOVERY.md](RE_COPY_RECOVERY.md).
> Detailed library/linker investigation:
> [API_IMPORT_RESEARCH.md](API_IMPORT_RESEARCH.md).

# Assessment of the latest import comparison

Input: eleven API differences supplied by the owner for Anadius and the
current MSVC DLL. No new native DLL or linker MAP file was provided.
The source-level evidence below is distinct from definitive identification
of callers at binary level. An import's presence does not prove its call
executes during normal startup, nor does its absence alone prove that a
corresponding high-level feature is missing. Suggested high/medium/low
priorities cannot be verified from this table.

| API | Source evidence and conclusion |
|---|---|
| GetFileInformationByHandle | The general copy body `048d20` and identity fallback `04895c` were reconstructed from the original dump. Avatar now calls the full body with mode 2. The legacy API is used in the actual native helper and its static import was confirmed in the current Release development DLL; a native MSVC rerun remains separate. |
| InitializeCriticalSectionAndSpinCount | Present in the earlier RE service mutex constructor. The active transport uses the newer owner-supplied Asio version, whose vendored `win_mutex` initializes an SRWLock. RE constructor sources can remain in the comparison/test graph; not every source body is linked into the application DLL. |
| TryAcquireSRWLockExclusive | Present in the zero/timed-wait branches of the original `FUN_18004a6d4` mutex helper. Also present in the newer Asio `win_mutex::try_lock`; unused inline/CRT branches may produce no import. Absence alone is not proof that locking behavior is missing. |
| VerifyVersionInfoA | Used for a runtime OS-version check in earlier IOCP code. The newer Asio `get_gqcs_timeout` omits it when `_WIN32_WINNT>=0x0600`. The Brownfield-derived `0x0601` target satisfies this condition. The application has a separate `VerifyVersionInfoW` hook/setup check on another path. |
| CreateFile2 | A direct relocation was found in the `__std_fs_remove` and `__std_fs_open_handle` bodies of MSVC 14.51 `libcpmt.lib(filesystem.obj)`. An isolated native library link reproduced the import. The current standard filesystem removal path uses this family. |
| GetExitCodeThread | Referenced by MSVC STL `cthread.cpp` / `_Thrd_join`. This is consistent with `std::thread` join/CRT dependencies; exact callers in the native DLL still require separate mapping. |
| HeapQueryInformation | Asio's MSVC aligned allocation/free path introduces an `_expand`/`expand.obj` dependency through UCRT `align.obj`. A test linking the official static library with aligned allocation imported the API; a plain malloc/free control did not. The owner's final DLL call-site map was not separately inspected. |
| InitializeSRWLock | Directly present in the `detail/win_mutex.hpp` constructor of the owner-supplied vendored Asio. |
| RaiseFailFastException | Called by `HookErrors::callback_exception` in `src/runtime/application.cpp`. The supplied Bluefield `src/windows/hooks.cpp` follows the same error/log/fail-fast/abort ordering. Not a normal startup call: it runs if an unexpected exception escapes a hook callback. The difference relative to Anadius is not a deviation from the selected Bluefield hook lifecycle. |
| ReadProcessMemory | `src/platform/crypt_query_host.cpp` safely reads the current process's UTF-16 filename and falls back to the selector if memory cannot be read. This comes from the protected reader in the owner-selected Bluefield eleventh hook. |
| WaitForSingleObjectEx | The MSVC STL `_Thrd_join` reference uses `INFINITE` and `alertable=FALSE`. The Ex-suffixed API does not imply an alertable wait. |

The initial comparison did not, by itself, add or remove APIs. The complete
copy body was subsequently reconstructed under the owner's later instruction,
as reflected in the current record above. Producing unused imports merely
to match lists does not establish behavioral equivalence. The owner-selected
new vendor/standard CRT and Bluefield hook dependencies remain unchanged.
A concrete behavioral difference must be examined through the corresponding
call path and error conditions.

Evidence paths: `src/platform/crypt_query_host.cpp`;
`src/runtime/application.cpp`; `src/platform/status_windows.cpp`;
`third_party/asio/asio/include/asio/detail/win_mutex.hpp` and
`impl/win_iocp_io_context.ipp`. Original RE and Bluefield donor files remain
preserved inputs outside the product tree. The MSVC STL thread reference at
`build/qa/MSVC_STL_cthread_reference.cpp` is diagnostic evidence, not a
complete binary map of the owner's installed MSVC version.

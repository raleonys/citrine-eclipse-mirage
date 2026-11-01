> This table represents the initial review snapshot. Following the owner's
> subsequent instruction, standard Windows I/O was restored and Gulrak was
> removed: [current status](WINDOWS_IO_FIX.md).

# Origin of the Windows filesystem difference

`gulrak/filesystem` is a portable library using the `ghc::filesystem`
namespace and supports Windows as well. It is not specific to macOS.
The backend choice of an Anadius macOS build cannot be established from
the supplied Windows DLL evidence.

## Available evidence

| Layer | Source / observation |
|---|---|
| Anadius Windows binary RE | `anadius-all-functions.pseudo.c` contains `__std_fs_open_handle`, `__std_fs_code_page`, and MSVC filesystem helpers |
| RE file source adapter | Supplied `anadius-file-source/native_io.cpp` uses `std::filesystem::path` and standard fstream |
| Yellowfield active I/O | `src/platform/file_io_windows.cpp` uses ghc paths/streams; runtime's internal `fs` alias is `ghc::filesystem` |
| Yellowfield deletion call | `Environment::remove_04948c` calls `fs::remove` during avatar-cache cleanup |
| ghc Windows deletion | `GetFileAttributesW`, then `SetFileAttributesW` when required, followed by `DeleteFileW` or `RemoveDirectoryW` |
| Anadius Windows deletion | Opens a handle and follows `SetFileInformationByHandle` disposition/basic-information branches with error fallbacks |

The difference arose from choosing ghc as the active file I/O and storage
backend during the Yellowfield runtime integration. Vendored library bytes
were unchanged; the backend called by the application changed.
Targeting the same C++ filesystem API does not establish byte-for-byte or
universal equivalence of Windows error, attribute, or sharing behavior.

Vendoring a library does not require migrating every Windows file operation
to it. Standard MSVC filesystem is part of the compiler's C++ library;
Gulrak is a separate implementation. This choice must not be presented as
a macOS-versus-Windows distinction.

This comparison did not itself implement a backend change. Its initial
scope corrected only the Brownfield Windows API compile-target definitions;
a file deletion/I/O backend change required separate behavior and error tests.

The later owner instruction was implemented: Windows I/O was reconnected to
standard filesystem/fstream, and Gulrak was removed from the product once
no active uses remained. The ghc table above describes the earlier state.
See [current implementation](WINDOWS_IO_FIX.md).

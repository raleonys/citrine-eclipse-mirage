# CRT invalid-parameter selection source

This module reconstructs seven bounded functions from the retained static REA/Ghidra evidence for `anadius64.dll`, SHA-256 `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`. It supplies independently compilable C++20 source, not recovered original source or a complete CRT/DLL.

| Original VA | Reconstructed behavior |
| --- | --- |
| `0x180052500` | Required context PTD lookup; required missing-PTD terminal dependency |
| `0x18005c5d0` | Optional context PTD lookup with saved Win32 error and state-index cache |
| `0x18005c63c` | Global table index selection; lazy index zero initialization preserves Win32 error |
| `0x18005c7ec` | Scoped context constructor, selection call, normal-return cleanup |
| `0x18005c888` | Thread handler first, decoded global handler second, required Watson otherwise |
| `0x18005c954` | Five zero argument call; a custom handler may return |
| `0x18005c974` | Five zero argument call followed by unconditional Watson if handler returns |

The handler ABI has three opaque UTF-16 string pointers, one 32-bit line number and one pointer-sized reserved value. Nothing dereferences those strings. PTD handler offset is `0x3b8`; global handler decoding is `ROR64(encoded ^ security_cookie, security_cookie & 63)`. The encoded cell is read before the cookie. A thread handler bypasses both reads.

`Context` models the observed 64-byte scoped context layout. Lookup with a null PTD may allocate through the mandatory backend `optional_ptd_064dd0`; failure is not cached as success. The state index defaults lazily to zero. The optional helper returns its captured lookup pointer after restoring Win32 error. The required helper tests that captured result but returns the context pointer read afterward, as the assembly does. Cleanup snapshots each saved error before its required PTD lookup and writes errno before doserrno. Tests deliberately mutate borrowed context during the fake SetLastError call to verify those ordering distinctions.

The wrapper reads the locale-changed flag and conditionally snapshots two locale pointers. Its selected helpers do not change locale mode to 2 or enable saved-error restoration; consequently those cleanup branches remain dormant for the bounded wrapper route. Their original operations are retained for explicit caller-supplied context: mode 2 clears bit 1 at PTD `+0x3a8`, then optional saved errors are restored at `+0x20/+0x24`. A mode-2 context requires a live nonnull PTD. Read/write operations use memcpy, permitting valid unaligned fixture storage without forged typed references.

All actual platform behavior remains mandatory: PTD/FLS allocation and lifetime, indexed global table ownership, cookie ownership, valid handler dispatch with native ABI/CFG/XFG, and both terminal policies. A caller can implement the thread source's `invalid_parameter_05c954()` dependency by forwarding to `invalid_parameter_noinfo_05c954(effects)`. This module does not infer a full combined native backend.

`watson_05c9a4` and `missing_ptd_05f03c` are required non-returning dependencies. Tests implement them with sentinel exceptions inside the synthetic test program. No real Watson, abort, process termination, native handler, module, original DLL or game is executed. Normal custom handlers may return; no fabricated failure or success result is inserted. Exception propagation from reconstructed C++ dependencies is available for tests; original SEH/unwind behavior remains unproven and must be supplied before a native ABI integration claim.

Run `./build.ps1` using the existing workspace LLVM-MinGW toolchain. The production object compiles independently with C++20, optimization and warnings as errors. The synthetic test executable passes **1,583 checks**, including all 64 cookie rotation amounts, high-bit addresses, encoded null, thread/global precedence, argument forwarding, noinfo/noreturn differences, null lookup retries, LastError preservation, unaligned PTD storage, context cleanup and exact snapshot ordering. See `validation.json` for compiler and output hashes; `evidence.json` and `assembly-evidence.txt` retain address and evidence references. Tests establish this source's bounded behavior; original runtime equivalence is unproven.

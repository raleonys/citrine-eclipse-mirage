# CRT termination source reconstruction

Ten local functions from `anadius64.dll` are reconstructed as C++20 normal control flow. The target SHA-256 is `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`. `evidence.json` binds each function to retained REA/Ghidra pseudocode and assembly Evidence IDs and the complete retained artifact hashes. `selected-assembly.txt` and `selected-pseudocode.c` contain bounded verbatim function excerpts. Ghidra names/signatures are analysis output, not the original source.

| Original VA | Source function | Recovered behavior |
| --- | --- | --- |
| `18005cb00` | `guarded_call_05cb00` | Lock supplied dword ID, lambda, reread separate unlock ID |
| `18005cb38` | `termination_lambda_05cb38` | Begun exchange, encoded callback, normal/quick table, terminator ranges, completion byte |
| `18005cc04` | `common_exit_05cc04` | Managed PE probe for terminal calls; guarded lock 2; return or terminal policy |
| `18005cccc` | `terminate_05cccc` | Optional TerminateProcess, managed exit attempt, ExitProcess |
| `18005cd00` | `use_terminate_process_05cd00` | Exact EAX comparison and AL-only policy test |
| `18005cd24` | `managed_exit_05cd24` | mscoree.dll/CorExitProcess lookup, conditional invoke and release |
| `18005cd9c` | `cexit_05cd9c` | Common core with `(status=0, mode=0, return_to_caller=1)` |
| `18005cdac` | `immediate_exit_05cdac` | Common core with `(status, mode=2, return_to_caller=0)` |
| `18005cdb8` | `c_termination_complete_05cdb8` | Full dword getter for begun cell `A4F08` |
| `18005cdc0` | `exit_05cdc0` | Common core with `(status, mode=0, return_to_caller=0)` |

`c_termination_complete_05cdb8` reads the **begun DWORD**, rather than the completed byte at `A4F18`. The lambda exchanges begun to 1 before invoking any callback. Returning `cexit` leaves the completed byte unchanged, so repeated direct `cexit` calls can invoke the encoded callback and initializer ranges again after the normal onexit table has been emptied. The outer Entry source checks the begun getter to decide whether it needs `cexit`.

Mode zero decodes the callback in `A4F10` using the current cookie, skips the encoded-null value equal to that cookie, then executes the existing reconstructed CRT onexit core for `A52A8`. Mode one executes the distinct `A52C0` quick table. Other modes skip both tables. After callbacks the lambda rereads mode: zero invokes the four-slot range `7F728..7F748`; every active mode invokes the one-slot range `7F750..7F758`. It rereads `return_to_caller` before setting the completed byte and call-local flag. Table execution uses the actual existing onexit source, including nested recursive lock 2, LIFO order, pre-invoke clearing and rescan after table mutation.

The PE probe mirrors the reachable reads in assembly: module base, `MZ`, sign-extended `e_lfanew`, `PE`, optional-header magic `20B`, unsigned NumberOfRvaAndSizes greater than 14, and nonzero CLR directory RVA. It performs no invented size or RVA checks. The new image provider enforces live bounded readable storage; unsupported memory faults and original exception handling remain outside this source contract. A terminal managed image can trigger two `CorExitProcess` attempts if that function returns, one before CRT cleanup and another in final termination.

Managed module lookup starts its output cell at zero, tests the full 32-bit BOOL and frees a nonzero written module even if the lookup returned false. The optional TerminateProcess result is ignored. A returning TerminateProcess continues through managed exit to ExitProcess. Policy `06C110` is compared using full EAX against 1; `06C09C` is tested using AL only. Synthetic terminal exceptions in tests stand in for the required nonreturning `exit_process` provider.

`State` borrows the same onexit state used by registration and the same encoded callback cell initialized by the cache module. Owners must outlive this state and every callback. The two CRT tables are distinct from the application's Entry tables. The caller must initialize both table encodings before use; zero-initialized source storage is not an encoded-empty table for a nonzero cookie. Cookie changes must be coordinated with every encoded cell. Providers must supply compatible recursive lock 2 and externally serialize mutable globals and callbacks. The begun atomic is a new C++ source synchronization choice for the original XCHG and read; its representation is not an original ABI-layout claim.

`Effects` requires every remaining callback, rebuilt mutable table-range binding, bounded image read, module/export lookup and terminal policy. Original preferred-image VAs select metadata only. Production code provides no fake success and never executes an original VA. The existing initializer core can be used to implement the range provider with caller-bound rebuilt spans.

Only normal paths are reconstructed. No RAII unlock, SEH filter, unwind cleanup or CFG dispatch policy is invented. A synthetic callback throw therefore propagates without an added unlock; this deliberately does not claim original exceptional behavior. ExitProcess is required to be nonreturning. No target DLL, game EXE, native module load or native process termination is executed by the tests.

Build and validate from this directory using `./build.ps1`. The existing isolated LLVM/MinGW toolchain compiles one production object and links the synthetic test program with the existing onexit source using C++20, `-O2 -Wall -Wextra -Werror`. The suite passed **1,148 checks** covering ordinary and repeated `cexit`, cookie decoding, distinct tables, all modes, AL/EAX distinctions, mutable callback-visible arguments, lock ordering, nested table execution, table replacement, managed-output failure cases, signed PE displacement, probe short circuit and synthetic terminal paths. This establishes the tested source behavior, not binary equivalence or a complete independently compilable replacement DLL.

# B9 — Indexed thread error route

The existing SourceServices::thread_error no longer delegates to an unresolved
lower provider. It calls throw_thread_error_04a648, which selects the exact
retained code/context pair and throws a rebuilt compiler std::system_error.
The helper adds one selected Services route: 12/17 bound and five remaining.
No new provider interface, native function-body claim or native closure is added.

| Index | Retained code | Exact context |
|---:|---:|---|
| 0 | 16 | device or resource busy |
| 1 | 22 | invalid argument |
| 2 | 3 | no such process |
| 3 | 12 | not enough memory |
| 4 | 1 | operation not permitted |
| 5 | 36 | resource deadlock would occur |
| 6 | 11 | resource unavailable try again |

04a648 indexes DWORD cells at 0x180087df0 and the pointer table at
0x180087db8, passes both to04a528, then invokes the native C++ throw helper.
All seven pairs were decoded from retained REA rdata
ev_1585c277e2b5dffb5a54c26a1db122db985c13d72fea8b0b6272dbea69b9b082.
The retained bundle and decoded bytes/addresses are hash-bound in
thread-error-evidence.json.

Category choice was checked independently rather than inferred from errno
numbers. Parent performed a static eight-byte REA query at0x1800a14e0:
0802088001000000 points to vtable0x180080208. Its retained name slot points
to0x18000fe60, whose pseudocode returns "generic". Evidence:
ev_108796666f1e134d971f55a1f575e33ff47f704541695016fd846180008ca5a9
and ev_a4d693e411afb626e5e7c3c3dd130c68e02975e25ba85568d15281ee8de1aa47.
Message slot0x18000f240 calls0x1800480d8; the complete pointer/name/message
chain is recorded. Target SHA-256:
9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c.

This supports selecting the rebuilt compiler's std::generic_category()
family. It does not establish original singleton pointer identity, native
std::system_error object layout/RTTI, translated category messages, what()
punctuation/locale, errno/LastError side effects or MSVC throw/unwind ABI.
Those remain compatibility boundaries. No original VA is called.

The supported input domain is0..6. Other indices throw std::out_of_range
before reading the table; the native unchecked out-of-range access is not
reproduced. The code and explicit context are preserved for valid inputs.
Compiler exception allocation may throw its own allocation exception.

Only three approved prior source files changed for this scope:
services.hpp, services.cpp, and the B7 policies_tests.cpp assertion.
The historical B7 test now checks retained code11, selected generic category
and explicit context instead of claiming the thread error remains lower-owned.
Other Services methods still delegate or use their previous concrete routes.

Strict C++20/O2/Wall/Wextra/Werror/static compile of the helper and Services
translation units passed. Independent B9 tests and the common build are
recorded by the test/delivery owners. This author did not read an original input,
execute/load a target or rebuilt DLL, modify original bytes, inspect actual
user CFG/token/achievement contents, or access a blocked version artifact.

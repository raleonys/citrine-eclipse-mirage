# Typed error/context propagation028240 — 2026-11-03

SOURCE_IMPLEMENTED_NOT_COMPILED_OR_EXECUTED.

The source counterparts of native028240 accepted a full socket Error but passed
only its number to fail(int,...). fail always constructed std::system_error with
the compiler system_category. Thus misc1/open became a Windows system error
instead of asio.misc/Already open. The IOCP constructor overload also ignored its
opaque context argument, collapsing distinct mutex084428 and port084528 identities.

OwnedServer now throws SourceError, a compiler std::system_error subclass that
owns the operation text and retains the complete typed source Error (value,
category identity and logical family). It also stores an optional numeric native
context without dereferencing/calling it. Existing std::system_error catch sites
remain applicable. Typed acceptor and service/IOCP raise028240 overloads now pass
the full Error. Direct SDK-only integer failure helpers stay system-category
projections because those seams provide no source category token.

The shared asio.misc category and exact020a80 value body moved to the existing
socket/native.cpp unit. Policy and OwnedServer now use the same compiler category
and message implementation; no Policy->OwnedServer lower-layer dependency or
duplicate body was added. Policy's previous function name remains available as
a source using-declaration. This relocation requires consistent rebuilding of
callers; it was not validated against old objects. Public SourceError is a new HOST
type, not a recovered MSVC std::system_error object/RTTI/unwind layout.

For typed source system errors, the compiler projection is std::system_category;
the complete source identity remains available separately. Native generic/system
category ABI is not inferred from an opaque token. The top ServerBackend::Error
seam carries only a logical family; its SourceError identity is explicitly0,
not a guessed native category address. IOCP what() retains the existing HOST label
"IOCP constructor"; the exact native literal bytes were not newly decoded.
Its two original numeric context identities are now separately observable.

Retained028240 pseudocode copies both words of the incoming native error pair
and passes the operation pointer to0248b0 before023990 throws.0248b0 copies the
C-string context and the error pair into its system_error construction route.
Complete assembly/pseudocode slices and parent hashes for028240/0248b0/023990/
024990 are in error-028240-retained/manifest.json. These are existing observations,
not new provider decompilation or an original runtime trace.

REA binary_session was called read-only: no target open, REA6.8.0 at the configured
path, Ghidra12.1.4 available with Java25.0.5. The complete response is retained in
error-028240-rea-session.json. No target was opened, no original binary accessed,
no database analysis or project build initiated by this check.

Unexecuted test sources cover misc/system category/value retention, operation
ownership, copying/catching through std::system_error, signed error bits and the
two opaque IOCP contexts. An additional own-child integration source injects a
failed CreateIoCompletionPort into actual024990 construction and checks the
typed error/context plus poisoned retained prefix, then exits intentionally.
The child has not been launched; no PASS or actual exit result is claimed.

Six production source/header files and two test sources changed; four were not
previously pending, giving59 current differences from the frozen1017-source full
build. No new native normal-body recovery or native ABI closure is counted here:
020a80 was relocated, and028240's selected source propagation was corrected.
No compilation, link, project test, DLL execution or ZIP update occurred.

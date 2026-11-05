# Context-free error028210 — 2026-11-03

SOURCE_IMPLEMENTED_NOT_COMPILED_OR_EXECUTED.

Retained native028210 copies the two-word error pair, constructs a system_error
through00a8b0 and throws through023990. Unlike028240/0248b0,00a8b0 obtains the
category message without receiving or prepending an operation string.

All three retained direct calls are:

| Caller | Call site | Condition | Selected source correction |
| --- | --- | --- | --- |
|024480|0x1800247d9|IPv6 parse failed, then IPv4 parse also failed|Remove invented `address` context|
|024480|0x18002480f|Successful local endpoint query reports size greater than128|Preserve0x2726 and remove invented `endpoint_size` context|
|029ea0|0x180029ff4|Context run returned a nonzero error|Remove invented `io_context::run` context; no normal cleanup follows|

The separate failed local endpoint query still calls028240 at0x1800247cf with
the native `local_endpoint` string; its source context is preserved.

The existing ServerBackend interface now documents empty context as the selected
028210 route. Existing Worker/Graph forwarding methods pass that string_view
unchanged. No new virtual method or general native category ABI was introduced.
OwnedServer dispatches empty context to a new SourceError(Error) constructor
using the compiler's context-free std::system_error overload. It retains the
typed source error and category token, with empty operation and zero context
address. Context-bearing SourceError construction remains separate.

The higher ServerBackend::Error seam still provides only a logical category;
its SourceError token remains explicitly0. Native generic/system category
identity, arbitrary category formatting, MSVC exception layout/unwind and exact
native what() byte equivalence remain unvalidated. The compiler's no-context
overload is a semantic projection, not a recovered MSVC exception object.
Selected existing028240 callers all have nonempty contexts; an arbitrary empty
native028240 string is not separately representable by this bounded interface.

Existing server test source now distinguishes parse/oversize no-context errors
from local_endpoint context, including exact values and logical categories.
A run-error scenario requires readiness already published, misc4 preserved,
empty context and absence of normal destruction. OwnedServer test source checks
typed metadata and compares context-free what() only to the same compiler's
context-free system_error, not to an executed original binary. These sources
were not compiled or run.

Complete retained assembly/pseudocode slices, parent SHA256 values, Evidence IDs
and direct-call records are in error-028210-retained/manifest.json. No fresh
binary/provider/runtime observation was made. Six source/header files changed;
62 source/header files now differ from the frozen1017-source full build.
No new native normal body, native ABI closure or total completion percentage is
claimed. No build, project test, link, DLL execution or ZIP update occurred.

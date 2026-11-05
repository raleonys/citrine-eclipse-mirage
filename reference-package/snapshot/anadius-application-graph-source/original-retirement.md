# Original Graph / Policy retirement — 2026-11-03

SOURCE_IMPLEMENTED_NOT_COMPILED_OR_EXECUTED.

A source review found a deterministic assembly conflict. Graph::create accepted
OriginalApplication with an empty external_drained slot. Graph::install_server_impl
then always filled that slot with its cooperative proof/Graph-drain wrapper.
Policy::create uses the shared profile validator, which rejects a nonempty
external_drained in OriginalApplication. Thus the actual original Graph/Worker
path could not reach server construction, even though its entry and initial
constructor composition were source-defined. No runtime test exposed this;
the finding comes from tracing the typed source conditions.

Graph now keeps the original external slot empty. It passes its actual local
Impl::drain through a separate Policy::GraphRetirement factory argument. This
callback performs existing session/job checks and real queue-string/Parent lease
retirement; it does not return an invented unload attestation. Policy first checks
observed worker return and actual tracked IO/work. It retains a false retirement
result and rechecks actual IO/work after true. Cooperative profiles keep their
existing external-proof/Graph-drain wrapper and reject this original-only argument.
Standalone original Policy construction can omit the callback when no Graph exists.

The factory parameter changes the HOST C++ Policy::create symbol and constructor
signature. All Policy factory callers must be rebuilt together. It changes no
recovered/native SDK entry signature and adds no field to the public Options
aggregate. The stored callback is retired while Policy metadata and backing live.

Uncompiled test sources now cover actual original Graph installation and known
unstarted close, a local false result, a true callback introducing tracked IO,
post-callback recheck, and cooperative rejection of the original-only callback.
The preceding original Worker no-milestone scenario also traverses this real
Graph/Policy installation and normal no-session retirement when eventually run.
No test, compiler or linker was invoked. No assertion count or PASS is claimed.

Both loader helper bodies remain present in source. This stage changes five
source/header files; 55 files now differ from the last frozen full build. The last
build, failed-link evidence, receipt and ZIP remain historical and unchanged.
Loader-lock execution, general concurrency, full native teardown, CRT/EH and live
FreeLibrary remain unvalidated. This is one source assembly correction, not proof
of a working production DLL or a measured overall completion percentage.

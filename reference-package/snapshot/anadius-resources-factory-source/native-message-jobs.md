# Native message jobs in the Image — 2026-11-03

SOURCE_IMPLEMENTED_NOT_COMPILED_OR_EXECUTED.

The actual Image Worker configure selected native socket/IOCP adapters and
OriginalApplication lifecycle, but left Graph ProfileSettings.jobs empty.
Profile::Profile then selected InlineJobs, so merely implementing NativeJobs
had not connected detached message execution to the production Image path.

Image now owns a second real NativeThreads provider from the same Ready Domain.
DomainStorage::make_threads runs on the original Domain controller, as required.
The configure closure retains this provider. On the actual server worker/fiber,
configure_native_worker runs and select_native_message_jobs constructs existing
owned_jobs::NativeJobs with the actual configured job capacity. Creating it on
that worker preserves its lifecycle affinity. NativeJobs construction is passive:
it starts no child or event, creates no IOCP and publishes no job.

The selector rejects missing/non-Ready Domain providers, incompatible lifecycle,
occupied jobs slots and unsupported capacities. It rechecks the destination after
construction before committing the owning provider, preserving LastError. It
does not substitute an inline fallback or reset an existing provider. Later
NativeJobs::bind_prepared validates its exact provider Domain against Graph's
actual prepared Parent/Ports; no arbitrary ownership token replaces that check.

The existing source path then remains Profile PreparedJobs -> NativeJobs -> real
recovered begin_thread05e8e0 and detach04a458 -> child handoff/process/return ->
IOCP receipt and physical thread/FLS reaping. The selector adds no stop/unload
protocol and does not change those algorithms. Default Graph users that leave
jobs empty still use the documented InlineJobs policy; the Image now explicitly
selects the native provider.

Existing static evidence in native-job-gap.md identifies message closure setup
at 0x180025526/02552d, thread entry021770 selected at02554b, begin-thread call at
025556 and detach at02557f. Assembly evidence is
ev_17a121443001ec46ed018ca1fad07bc2cfd74c2e067c1f3ccf0bb7bd84b95c31;
detach evidence is ev_218bf3eda319b5a7858921c077bd0fd00cadc3c2d550d8952be3eef945886923.
No fresh binary analysis or original execution occurred here. Existing static
observations do not establish original runtime interleaving or native class ABI.

Uncompiled test sources cover passive selection, wrong/missing provider/profile,
zero/oversized capacity, occupied-slot preservation and Fresh NativeJobs/no child
start. The existing original fresh Runtime constructor scenario now retains the
same native message-provider capture. It still does not run application startup.
No source scenario was compiled or executed in this stage.

This closes a concrete missing Image selection in source. It does not establish
successful link, real message-thread execution, full-session retirement, native
concurrency, CRT/EH or DLL unload equivalence. Both loader bodies remain source-
defined; 55 C++/header files differ from the frozen full build. Build/link/test
results, package receipt and ZIP remain unchanged. No reliable overall DLL
completion percentage is derived from this selection.

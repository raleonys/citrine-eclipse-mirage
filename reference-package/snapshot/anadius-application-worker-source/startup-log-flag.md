# Runtime ShowLSX to message logging — 2026-11-03

SOURCE_IMPLEMENTED_NOT_COMPILED_OR_EXECUTED.

Following the native message-worker route exposed a separate missing input.
BootstrapConfiguration already writes the RuntimeOwner ConsoleState show_lsx
byte through the actual startup argument/environment pass. SourceBackend retained
that cell, but CallbackStateView contained only configuration/runtime/execution.
The Image's Worker configure therefore left Graph show_lsx at its default0, while
direct Graph integration fixtures supplied1 explicitly. Thus enabling the startup
option did not select message received/response/event logging in the Image source.

CallbackStateView now appends an optional const byte pointer. SourceBackend
provides its exact borrowed ConsoleState show_lsx cell when binding providers.
Actual OriginalApplication Worker reads it after configure, after startup setup,
and before Graph creation. Graph/NativeLogic/NativeMessageContext retain their
existing log gates; no logging algorithm or source message body changed.
Other profiles keep their configured Graph value. Legacy three-field source views
default the pointer to null and retain their configured value as well.

Retained startup-audit.json maps ShowLSX to 0x1800a3ef0. The source chain is
BootstrapConfiguration::references -> RuntimeOwner ConsoleState -> Backend view
-> Worker -> Graph Options -> NativeLogic Dependencies -> NativeMessageContext.
The native_message.cpp body tests that byte before received/response/event logs.
No fresh native observation or original binary access occurred.

This is a startup snapshot for the current Graph. It does not implement later
live mutation of a native global during active requests. The real RuntimeOwner
must outlive its borrowed view, as the existing Image lifetime already requires.
Appending the pointer changes HOST CallbackStateView layout/calling ABI: every
factory/view consumer requires a consistent rebuild. Old aggregate source initializers
remain valid, but old objects cannot be linked against the new layout.

Uncompiled Worker scenarios now check exact Backend cell identity, a value changed
after passive Worker construction and read before first original dispatch, and
preservation of Explicit's different configured value. The isolated dispatch case
still exits at observation rather than claiming normal cleanup. These scenarios
do not assert observed console text or a native child-message log; none was run.

Three production source/header files and one test source changed. There are still
55 files pending versus the frozen full build. Both loader bodies and native Jobs
selection remain source-defined. No compilation, linking, project tests, DLL or
ZIP execution/update occurred. No overall DLL completion percentage is inferred.

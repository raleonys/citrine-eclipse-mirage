# Application and tooling ownership

This scope does not redesign the runtime architecture. The current RE application
sources reside under
`src/{config,core,runtime,net,platform,protocol,router,handlers,codec,hash,xml}`,
with headers under `include/citrine_eclipse_mirage/`. CMake uses explicit source lists and
does not add glob patterns or include paths into research or frozen roots.

`tools/` and `scripts/` form the operator and QA layer; Golden Python,
PowerShell, and test fixtures are not linked into the application DLL. Mock
operating-system providers remain confined to test targets. The first-party
`citrine_eclipse_mirage` namespace is preserved.

The intended architecture retains distinct boundaries for ABI ingress,
application/domain logic, narrow ports, Windows/transport/storage adapters,
LSX/XML framing, and bootstrap/teardown ownership. The current RE function
collection is not claimed to implement every intended architectural component.

Automatic Application/Service ownership, production Asio/filesystem bindings,
and the conditional 11-hook rollback/drain path have been implemented. Network,
configuration, hook, and blocking teardown work runs on a worker thread, apart
from DllMain preparation. The application's three C exports are distinct from the
C++ development DLL; [PUBLIC_DLL_ABI.md](PUBLIC_DLL_ABI.md) documents the
current contract. Actual Windows/native quiescence and game acceptance require
separate evidence; Wine tests do not establish either.

The current Config2/VDF byte grammar is compiled with PEGTL under the
`citrine_eclipse_mirage_vdf_objects` owner target. Both the actual DLL and
DocumentPipeline use this parser. Source/action coverage and regression
boundaries are recorded in [VDF_PARSER.md](VDF_PARSER.md).

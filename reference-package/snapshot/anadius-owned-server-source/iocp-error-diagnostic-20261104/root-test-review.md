# Independent test-source review

Result: no blocking source finding in the authored IOCP diagnostic assertions. No compiler or test process was run.

Reviewed server_tests.cpp SHA256 **d2895d9128410aa015cdbb36e1288b716f540b718a4ca7c331290f1361b5f4f0** and server_integration_tests.cpp SHA256 **103e830ec37f2d052c94a7410c079146e9fd5f72cb03f1c199b97406a2fcffec** against their stage/before copies and the production delta.

Direct cases cover both measured addresses, INT_MIN signed value retention, native category/name, owned operation/source metadata, complete what text, ordinary exception copying/slicing and base catch compatibility. Separate cases preserve unknown-address, address-free, context-free, non-system and misc behavior. Numeric addresses never override caller-supplied operation text.

The existing retained-prefix port-error child now expects iocp and the native category. The added gate-error child injects failure at the first scheduler critical-section initialization, sets error8 and returns FALSE without initializing the physical section. Existing initialize_gate captures that code;024990 obtains the category before raising. Assertions distinguish one category/no port attempt from the port-failure route's two categories/one attempt. Both verify retained initialized prefixes, poisoned backend, exact metadata and what before ExitProcess. No guessed cleanup or invalid-section destruction occurs on success of the assertions. Child entry modes remain own future test processes; none was launched during review.

Each expected complete diagnostic uses one independent default-native SDK oracle before the selected operation. This checks provider composition without a hard-coded localized OS string. The fixture does not intercept SourceError's SDK calls, so it does not measure their count or establish once-only formatting independently. Copy assertions likewise check owned values, while the previous compiler/provider records supply copy/format behavior evidence.

Existing unchanged iocp_constructor_tests.cpp covers lower-constructor NULL-port/error0 fallthrough and DWORD/high-bit conditions. The owned backend's pre-existing lifecycle validity checks are outside that lower-constructor claim. No new zero-code public SourceError parity, successful failing-prefix teardown, native MSVC ABI/EH/RTTI, or whole-DLL behavior is certified.

Source assertions remain uncompiled and unexecuted pending an explicit rebuild request. Source review is not a PASS result.

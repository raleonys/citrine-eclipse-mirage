# GF02 — Byte-identical HF19 Receive/Framing/Codec Adoption

Classification: `SOURCE_ONLY / NOT_RELEASE`; canonical HOTFIX19 is not modified.

Actual adopted production codec: AES-128 block, message codec, session crypto, LSX frame scanner.
Actual adopted **debug/native/test** support: receive buffer and frame ingress. This distinction is deliberate: importing test ingress does **not** mean the production Asio transport is connected.
`request_parser.hpp` is imported as a type-only dependency for TextSpan; `request_parser.cpp` and TinyXML2 parser were **not** linked into GF02, and route XML parsing is not asserted.

All 13 source/header modules and 6 native upstream unit-test files are byte-for-byte preserved from immutable official HOTFIX19. `OwnedIngress` is a *new* minimal single-thread owner-safe integration adapter: stores ingress state, copies full frames to caller buffers, and consumes only after an adequate buffer is supplied. It is not wired into DLL exports or WinSock/Asio. `ResourceOwners` GF01 model is exercised with injected cleanup failure and retry; no actual Win32 handle is adopted.

GF02 acceptance: portable GCC/Clang CTest including original HF19 unit tests and owner-safe integration, sanitizer QA, origin SHA checks, tamper/clean-extract package QA. Windows MSVC x64 exports/native/runtime still NOT_RUN; LSX network service remains NOT_IMPLEMENTED.

Next target GF03 is to wire the source-owned session lifecycle/transport with a safe request boundary under explicit source test, then real Windows x64 MSVC tests before asserting native acceptance. Never infer network success from unit tests.

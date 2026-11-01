# GF12 FIX01 — verifier safety hardening (NOT_RELEASE)

- Parent source: immutable GF11 SHA-256 `962d4d636f8dd666e84486336d3b378346c05136f78e1879c07fa09f78b1659e`.
- Canonical source: immutable official HF19 SHA-256 `c7e16beac0618c7f364810261f8377fdfcfb7499d184a603615690ec269e24ab`.
- Pre-FIX01 development source ZIP SHA-256: `75b934924806884b49df8857113214bf8dd648a0a20888f26188abb33e0b34df`.
- Finding: fail-closed source verifier inspected only the first `target_link_libraries(gf_preloader PRIVATE ...)` invocation, so a forged manifest accompanied by an appended second shipping link command could be accepted.
- Fix: verify every `target_link_libraries` invocation targeting `gf_preloader`; exactly one invocation is permitted, with `PRIVATE gf_core gf_adopted` and no other operands. No shipping linkage added.
- Regression: `scripts/test_gf12_guard_negative.py` must reject all six mutations for the intended reason, including rehashed appended shipping-link injection. Run using `PYTHONDONTWRITEBYTECODE=1` to avoid generating forbidden `__pycache__` files in the verified source directory.
- HF19 original six GF12 files remain byte-identical; no canonical/HF19/GF11 source, release pointer or ABI change.
- Original GF12 native/IOCP/real LSX/Consumer/game and shipping final link remain **NOT_RUN**. Linux full dispatcher link blocked solely by Windows-only `normalized_avatar_path`.

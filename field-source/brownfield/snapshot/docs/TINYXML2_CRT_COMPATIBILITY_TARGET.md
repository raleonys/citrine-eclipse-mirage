> **HF12 current static CRT supersession, 2026-10-13:** Shipping `/MT`, Debug `/MTd`, pending exact Windows acceptance. Historical `/MD` or `/MDd` statements below are lineage only. See `docs/CRT_STATIC_POLICY.md`.

# HF11 TinyXML2 — compatibility-only UCRT link closure (Candidate10)

**Status:** Experimental, source-only remediation candidate. No Windows Candidate10
link, PE binary comparison, CRT scenarios, full eight-shard QA, or game acceptance
has been performed. HF10 remains the official canonical release.

## Evidence / unchanged requirements

Windows Candidate05 CRT Transition stopped while linking its *historical
comparison-only* `/NODEFAULTLIB /ENTRY:DllMain` DLL. Its production-normalized
`/MD` build is a separate target. The linker reported unresolved `strcmp`,
`strlen`, `strncmp`, `isalpha`, `isdigit`, `isspace`, `fopen_s`, and related UCRT
symbols from TinyXML2 and Store. Existing comparison link closure explicitly
contained `msvcrt vcruntime` but not `ucrt`.

Microsoft's supported post-VS2015 split CRT import libraries for `/MD` with
`/NODEFAULTLIB` are **msvcrt.lib, vcruntime.lib, ucrt.lib**, not only the first
two. References: Microsoft C++ Team Blog "Introducing the Universal CRT"
(https://devblogs.microsoft.com/cppblog/introducing-the-universal-crt/),
Microsoft Learn "C runtime (CRT) and C++ standard library (STL) .lib files"
(https://learn.microsoft.com/en-us/cpp/c-runtime-library/crt-library-features).

## Bounded change

Only the `PRELOADER_L_COMPATIBILITY_CUSTOM_ENTRY=ON` branch in `CMakeLists.txt`
sets explicit `/MD` runtime selection and adds the **dynamic UCRT import library**
`ucrt`. `/NODEFAULTLIB`, `/ENTRY:DllMain`, `/Zl`, legacy freestanding memory
helpers, security-cookie comparison, `/GS`, and the production-normalized
`/MD` path remain unchanged. This does **not** add any custom parser, shim,
new production dependency, import allowlist exemption or manual CRT startup.

`preloader_l_tinyxml2` stays the same SHA-pinned vendor source with the same
`/MD` configuration. `ucrt` is confined to the historical comparison option:
the normalized shipping and debug link target lists are unchanged.

The comparison DLL is only a **read-only binary-surface comparator**. It
bypasses regular CRT startup and is not supported for game loading, native
execution or release distribution. The Windows linker may still fail, and
no result is pre-accepted. The source checker includes mutation negatives
for missing UCRT, static `libucrt`, removal of the legacy flags, spillover to
shipping, and loss of the TinyXML2/Debug bindings.

## Acceptance boundary

1. On Windows, build exact Candidate10 with `build_crt_acceptance.cmd`. Observe
   comparison-only link first, normal CRT shipping link, and all subsequent
   binary/Native/CRT checks **without weakening any gate**.
2. Retain the full stdout/stderr logs, binary hashes and toolchain provenance.
3. Do not infer whole-release acceptance from CRT alone: complete eight-shard
   source QA, fresh extraction, Golden/FALCION, and exact CollegeFB27/Madden27
   acceptance separately.

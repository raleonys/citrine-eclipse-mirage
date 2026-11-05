# System category message020950 source correction

2026-11-04. SOURCE ONLY: no compiler, project test, linker, reconstructed DLL or ZIP operation. Original binaries and historical build/delivery proofs remain unchanged. The preceding tracked-executor stage is retained as its own source milestone.

## Observed native behavior and source change

Six fresh REA6.8.0/Ghidra12.1.4 records establish028620→objectA1110→vtable84248→message slot+10=020950 and its exact275-byte body. See native-findings.md and the full replies. Retained historical triage was only the seed, not substituted for these new observations.

Previously the selected Policy's system category message callback delegated to the compiler's std::system_category().message. Its exact behavior was not established by this source reconstruction. The callback now uses explicit system_message_020950 with injected/default FormatMessageA and LocalFree SDK imports. This change removes that compiler dependency for the actual selected category message slot; it does not claim an observed difference on every Windows installation.

The helper passes flags0x1300, sourceNULL, the input DWORD code, language0x400, an initially-null allocated-buffer pointer, size0 and argumentsNULL. It uses the returned count only to remove at most one LF and then at most one CR. Zero count or trimming to zero selects exactly `asio.system error`. Success uses a subsequent C-string NUL scan, preserving narrow bytes; it does not use the API count as the copied string length or strip arbitrary whitespace. Both normal paths build the result before LocalFree(saved pointer), including a NULL failure pointer; the free return is ignored.

Policy keeps the exact source message state across opaque effects. Unknown Format/copy/free/checkpoint outcomes retain the entered prefix and existing Policy poisoning; there is no added RAII free, retry or unload condition. Only host admission checks preserve their own LastError metadata effects. The SDK effects themselves remain visible, and the helper does not fetch a separate SDK error to construct the fallback. The final checkpoint precedes clearing the retained state, so a failed post-effect check cannot erase that prefix.

The explicit format_override remains first. Generic and misc message paths retain their previous implementations. The new defaulted imports are a trailing Options field; all dependent host layouts still require a consistent explicitly authorized rebuild. Compiler std::system_error category/what formatting, native MSVC string representation and EH/unwind remain separate unresolved ABI boundaries; they are not silently relabeled as solved by this message callback.

## Source validation and limits

Native helper cases use deterministic SDK callbacks, exact argument/pointer checks, CRLF/LFCR/repeated suffixes, embedded/leading NUL, return-count versus C-string-length differences, narrow bytes, fallback, NULL/nonNULL LocalFree and ignored free status. A free callback overwrites SDK storage to check that the returned owned text was already copied. Unknown effect cases retain state without inventing native allocation/unwind behavior.

Actual Policy read/write source cases clear format_override and exercise the selected system category through existing025480/025a70/ErrorContexts, observing prefix/append/flush ordering and Parent string retirement. Existing explicit formatter and misc tests remain distinct. These are unexecuted test sources, not observed OS or DLL behavior.

The native helper assumes valid FormatMessage storage/count/NUL output, matching the native dereference domain. Arbitrary malformed SDK output is not promoted into a new fallback. No runtime, locale matrix, compiler or whole-DLL equivalence is established. No completion percentage is inferred.

Four production before-images were reconstructed after their initial capture step was missed: native files from the read-only historical ZIP, Policy files by reversing only this stage's hunks. Each recovered byte sequence is checked against its authoritative prior source-snapshot or previous reviewed source SHA. They are labeled reconstructed baselines, never contemporaneous captures or rewritten historical validation. Test before-images were captured before their edits.

Final source hashes, independent reviews, fresh Evidence IDs, pending-source count and unchanged frozen-artifact proofs are recorded in evidence.json.

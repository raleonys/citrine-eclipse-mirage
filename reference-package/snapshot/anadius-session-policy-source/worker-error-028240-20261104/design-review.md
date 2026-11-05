#028240 diagnostic-value design assessment

Read-only source/native design assessment, 2026-11-04. No production changes, new REA queries, compiler, project tests, linking, DLL execution or packaging.

## Recommended bounded choice

Preserve the existing std::system_error catch/code contract. Correct the recovered84530 context literal in the two actual Policy stop raise projections to `pqcs`, and explicitly leave complete thrown what() native-value equivalence OPEN. Do not replace compiler_category(1) globally with a new error_category in this stage.

This is a partial literal correction, not completion of028240 or of the020950 terminal message route. It fixes a measured input value without silently changing category identity, exception ownership, injected SDK selection, or terminal permission. It should not count the remaining what() gap as solved.

The selected compiler system-category provider's equivalence to native020950 is UNPROVEN. Its bytes/effects have not yet been inspected or executed, so this assessment does not assert that its returned message is known equal or known different. The measured pqcs-versus-post literal difference is independent. The next bounded evidence question is the actual selected LLVM library's system_category implementation/static artifact, before choosing any category-contract migration.

If full visible what() equivalence is selected immediately instead, a scoped preassembled-diagnostic derived std::system_error is the feasible alternative that preserves the base code() identity. Its extra compiler base-construction behavior and the terminal diagnostic permission path must be explicitly bounded; it is not justified to disguise either with a global category.

## Native requirement

The fresh seven-record native audit establishes028240→0248b0→00a270:

- copy context C string;
- append `: ` only when that copied context is nonempty;
- invoke the supplied category's virtual message slot+0x10 using low32 code;
- append returned message by its stored count;
- copy combined text into the native exception text and preserve the original error pair.

For84530, the exact context is pqcs. For the selected native system category, the prior fresh binding proves that message slot is020950. Neither constructor substitutes a compiler system category nor appends a visible numeric/category-name suffix in its observed normal text assembly. Native class layout, exception copy ABI, throw/catch ABI and unwinding remain outside this value proof.

## Existing code() is an actual contract

Current Policy test source catches `const std::system_error&` and checks category address equality, not merely a logical enum:

- workerror: `&error.code().category()==&std::system_category()` (policy_tests.cpp, retained workerror case);
- write/worker terminal: explicit compiler system/generic category address comparison;
- owned-server typed SourceError tests preserve system-category identity and signed raw code, while misc uses the existing stable misc_error_category identity.

SourceError also exposes its separate typed original category token; compiler code() and native source metadata are distinct layers. A new global category named asio.system changes code().category(), name(), equivalence behavior and code().message() for every changed user, even if it keeps the same integer and remains catchable as std::system_error. That is not preservation of the current code contract.

The installed local libc++ header (`include/c++/v1/__system_error/system_error.h:27-40`) shows system_error stores a private error_code, code() is nonvirtual and returns that object. Hiding a derived code() accessor cannot fix a caller catching the existing base reference. There is no public setter to construct with one category then swap the base code to another. Such mutation, temporary global/TLS substitution or per-error category-identity tricks are excluded.

## Why the proposed global NativeAsioSystem category is not aligned

1. It changes the actual stable compiler category identity described above. Changing compiler_category(1) also affects write errors and other wrappers beyond the freshly proved84530 stop path.
2. A stateless global with real default SDK imports bypasses PolicyOptions.system_message injection. It would make normal logged system messages and terminal system messages use different selected lower providers, and controlled source cases could not attest the actual terminal SDK path.
3.020950's accepted adapter retains SystemMessageState on a throwing SDK/allocation/postcheck. A stack state inside error_category::message loses that retained pointer/phase on exception. Adding unconditional RAII LocalFree would invent native EH cleanup, expressly unproved by the fresh body.
4. A per-Policy/import-owned immutable category can reuse that Policy state/import selection, but system_error's error_code stores a category reference, not shared ownership. Exceptions/code copies can outlive Policy. A derived lifetime guard or permanent category registry would then be required, and category identity would still change. This is unnecessary ownership machinery for the currently bounded literal correction.
5. The std::system_error constructor's composition is out-of-line in this installed libc++ header. The native constructor's exact behavior does not prove this compiler implementation composes its what() with the identical byte/effect sequence. Do not claim exact native composition merely by choosing a category name/message callback.

These are concrete source/API constraints, not extra unload conditions or a claim that changing category semantics is always forbidden. Such a change would be a separate explicit HOST contract migration, not a transparent fix.

## Full visible diagnostic alternative, if selected

A small typed HOST exception derived from std::system_error can retain its original error_code category/value and own a preassembled diagnostic string. Override virtual what() to return that owned immutable value. A separate original source Error/context field can be retained, as SourceError already does, without changing .code().category(). This is a diagnostic-value projection, not a native exception ABI implementation.

Build `pqcs: ` plus exactly one selected020950 value before constructing this exception, using Policy's actual imports and caller-retained SystemMessageState. Do not route through format_override unless explicitly choosing HOST override semantics for this path; native00a270 calls the actual category message. Use the original/misc/generic selected category only where that caller was proved, not by mapping every raw logicalsystem token to native asio.system.

The standard base still must be constructed with the preserved compiler error_code. Its constructor may perform additional compiler category formatting/allocation; this header does not prove those internals or their SDK effects. Overriding what() prevents a duplicate visible suffix, but does not remove those base effects. Therefore the resulting claim could be exact visible diagnostic value plus preserved base catch/code contract, not exact native constructor SDK/allocation/EH effects. No reinterpret cast, private-field mutation, temporary error-category or TLS/current-constructor mechanism is acceptable.

## Terminal permission is a real integration point

This full-value alternative cannot safely call the ordinary logging helper through Policy::text after poisoning:

- Original Profile::Worker::raise validates then poisons Profile before calling Policy::raise. Policy::terminal performs its narrow source check, poisons Policy and clears its ordinary active reservation before the present throw.
- The tracked finish raise currently has environment_work_active and a live original/child proof, then poisons Policy before constructing the compiler exception. Its ordinary call.admit requires Bound, so reusing it after poison would reject.

A full adapter must capture and retain the exact terminal Parent/actor/JobPermit authority before intentional poison, construct the diagnostic under a narrowly defined terminal-only check, retain SDK prefix on failure, then throw. It must not unpoison/reopen normal Policy/Profile admission or call the stored ordinary checkpoint that now rejects the deliberate poisoned state. The original and tracked-child pathways must share the same selected message provider and completed work/flag prefix. This is a bounded future integration task; a global error_category does not supply that proof.

## Minimal next edit and validation scope

For the recommended literal-only step, change only the two84530 stop throw contexts from post to pqcs, and adapt their diagnostic-prefix expectations. Keep write's separate84720 context and all other SourceError labels untouched. Preserve std::system_error and integer/category address checks. Record explicitly that compiler-generated message suffix remains OPEN and020950's earlier logging correction does not finish it.

For a later full visible-diagnostic step, source cases need exact original and child terminal admission, actual injected FormatMessage/LocalFree called once for native text, no extra visible compiler suffix, preserved caught-base code/category, what() lifetime after Policy owner retirement, state retention on SDK/string/postcheck failures, and unchanged consumed work/flags. Compiler base-constructor side effects would still be a named ABI/projection limit unless separately resolved. No tests were run for this assessment.

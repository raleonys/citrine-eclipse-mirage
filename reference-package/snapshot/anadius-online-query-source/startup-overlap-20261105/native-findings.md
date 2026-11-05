# Original Online startup/query overlap

**Confirmed caller-side source defect, with a bounded ingress scenario:** the selected OriginalCrt Online bridge rejects an otherwise eligible avatar query while the first startup profile getter is active. The measured native callers publish online before those getters and permit the QueryImage virtual call without an OnlineReady check. This does not establish the external DLL's internal thread safety, full foreign ABI or a runtime occurrence; it establishes an additional source-side rejection/poison path absent from those native caller bodies.

Fresh static REA6.8.0/Ghidra12.1.4, own isolated session. binary_session verified aligned configured server and supported provider before open. Immutable original `C:/Users/Skysect/Downloads/anadius64.dll` SHA256 at open and after close: `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`. Twelve full records/187323 bytes/zero unknown records exported, session closed and post-close open=false/cleanup_pending=false verified. No original writes, annotations, source/test changes, build/test/link, target/process execution, network, ZIP or version.dll access. Full replies/EIDs/digests/limitations are in `native-evidence-index.json`; source authority is frozen in `source-hashes.json`.

## Actual production selection and source failure

The real image's entry.cpp calls `online_factory::select_native_original_online`; this selects concrete Online and module-reference leases, not the optional legacy OnlinePlatform lower-provider fallback. configure_native_worker selects OriginalApplication, and application Worker's original startup barrier does not add an Online/Host completion wait. Concrete application-effects Factory requires the exact actual Online; Content's typed avatar request reaches Online.avatar→Binding.avatar→QueryUse on the adopted object.

Current `online-source/online.cpp` sets online=1 before copy_username, copy_persona and copy_user_id. The first getter's copied result is assigned to the configuration only after that call returns. While it is inside a slow valid foreign username function, the source object/module are adopted and stable and the existing profile/path fields have not begun their success-path writes.

The facade's Online::Impl::Call permits concurrent Open calls by counting them and releases its metadata mutex before the lower call. However, `online-object/object.cpp` Binding::begin_object rejects `active_` unconditionally, sharing that Boolean with all foreign queries. Startup copy_getter holds it across its foreign call and owned copy. A concurrent valid avatar query therefore throws before invoking slot28. Online::invoke catches that exception and poisons the whole facade; the first otherwise successful getter can then fail its own completion check too. This is a reachable extra failure, not a missing function name or an allegation that HOST compatibility forwarding is production.

## Fresh native facts

| Body | Exact observation | Instruction Evidence |
| --- | --- | --- |
|035bd0|036002 calls status slot08; zero status→03600d writes online byteA3C3B=1;036039/03606c/036098 then call username/persona/userId slots10/18/20|`ev_3a2801694e3872418c4859c91164e01da057fe0cba1eb3ee02140104a090c697`|
|035b60|035b64 sets ECX1;035b69 calls wait036880;035b6f calls035bd0; only after it returns035bb5 selects OnlineReadyA3C3C and035bc0 tail-calls036820|`ev_38484aa68e3db6a0c85422e79708bdc058ec5d83302c1a932941a342607b8ebf`|
|036880|036916/03691d/036925 load third/server/online flags;036934–03693c makes phase1 depend on ServerReadyA3C3D, phase2 selects OnlineReadyA3C3C|`ev_9f1de4375f3c44c8e0c59d95b90042240c92853434c9a941fc374236aa1bc138`|
|0314f0|0316fe tests onlineA3C3B, not OnlineReady; eligible branch loads objectA3A88, resolves vtable+28 at031a65 and calls031aad with object/cache UTF16/owner/width÷208;031aaf consumes AL. Complete599-instruction body contains no036880 call|`ev_a6f6051699803b87a0d507bfa7f6abf6363033dbb10bfa5733cee99fc69aeebb`|
|024480|0245c4 completes acceptor construction, then024725/02472c publish ServerReadyA3C3D;024758 submits accept. Source equivalent binds/listens before this publication|`ev_4e8773ec4bcaae1dbadc4f0ef3ba6465ed4df416f35cdf6cc0f9d1a5444a8d2f`|
|00fd00|Recognized non80/443 route sets ECXffffffff at00fd5f and calls wait(-1) at00fd64 before normal3216 redirection;80/443 branches skip that wait. Original-only family/length branch tail-forwards|`ev_839077f99c02612bc8d409358a72a00b4078cf9a201c931361fd8e1c4381c349`|

The matching six pseudocode records are retained separately. Pseudocode type/terminal-call limitations remain provider observations; exact argument widths and control ordering above come from instructions. No external anadius64online.dll body was opened or executed.

## Valid scenario and essential qualification

1. The actual listener has been constructed and ServerReady published. A client that does not traverse this process's connect hook establishes a valid LSX session; alternatively a proved no-wait hook route may apply. No invalid object, null returned string or forged procedure is required.
2. Online startup authenticates successfully, writes online=1 and enters its **first** username virtual getter. That foreign call takes long enough for another request. It has not returned, so configuration username/persona/userId success writes and later avatar selection have not begun. This avoids basing the scenario on an unrelated concurrent std::string mutation.
3. An eligible request such as QueryImage with `ImageId="avatar:remote"`, `Width="416"`, `Height="208"` reaches the online avatar branch. The selected source Content/Parent gates do not serialize it against the distinct Online-startup actor. Native0314f0 would issue slot28 with the adopted object. The current Binding instead rejects the active startup getter and causes Online poison.

**Do not generalize this to every first game connection.** The ordinary recognized hooked3216/nonweb path waits for all three startup flags and therefore masks this first-startup window. Fresh00fd00 records that counterexample. The listener ingress itself has no universal OnlineReady barrier: an external/unhooked client remains eligible, and configured actual port80/443 is a separate no-wait case. This is a possible valid native caller interleaving, not an assertion that usual hooked3216 necessarily exhibits it or that it was observed at runtime.

Whether the foreign SDK internally serializes, accepts or rejects simultaneous calls is unknown. Preserving native caller behavior means not replacing its virtual call with the bridge's unconditional busy exception. A fix should address this selected Original caller policy without adding an all-ready wait, guessing SDK internals, removing lifecycle ownership or changing unrelated HOST contracts. No implementation is made in this audit.

## Scope of the initial bounded path audit

Concrete selection, load/exports, version, create, destroy, status/getters and application avatar/auth typed boundaries were checked before this candidate was pursued. Their retained caller shapes are implemented; old documentation of optional lower forwarding is not evidence of a present production stub. Full foreign class layout/RTTI/EH ABI, true SDK bodies, arbitrary pointer lifetime and complete concurrency/runtime behavior remain unverified. The present finding is specifically the selected startup-query overlap rejection. Source hashes identify the pre-fix code; they are not a rebuild proof or a claim that the whole DLL is complete.

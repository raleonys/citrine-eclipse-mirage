# Legacy configuration input-route audit — 2026-11-05

**Bounded negative:** no legitimate original CFG file/content/option was found that selects the source's collection-rejecting legacy document bridge. The actual image path always uses the full mutable FilePass/hash application route for scalar and collection/object inputs. `legacy` here names a historical HOST interface/composition, not an observed second original file dialect. No fresh REA question or production patch is justified by that interface's restrictions alone.

Read-only source and retained-native review. No user CFG contents, binary execution, build/test/link, packaging, source/status edits, new REA session or pending teardown edits. This report does not establish arbitrary parser/allocator/EH/locale equivalence or all configuration semantics.

## Actual source producer and route

1. `resources-factory-source/configuration.cpp:6–13` installs Configuration plus an owned scalar legacy reader. Ownership of that reader is required by the historical Dependencies interface, but construction performs no read.
2. `bootstrap-owner-source/owner.cpp:22–23` always constructs BootstrapFacade as `config_startup::BootstrapConfiguration`. Its `load_configuration_01c870`, `config-startup-source/startup.cpp:25–26`, calls the file-based startup initializer.
3. That initializer creates **FilePass** at `startup.cpp:8`; the pass at lines4–5 delegates to `config_application::load_and_apply`. It has no content/type/version detection selecting the old `Dependencies::load_document` method.
4. `config-application-source/application.cpp:143–151` chooses only the base/override filename (`anadius.cfg` versus `anadius_override.cfg`), reads raw bytes and parses through `read_config2` (lines23–28), then applies the selected root. Missing Config2 gives the logical empty root, not a legacy fallback.
5. `apply_config2` uses the same reusable section destination and separate record destination, performs Game→Emulator→User before the override return at line95, and for the base applies Content (97–104), Entitlements (105–112), and achievement name/total/Steam override child tables (114–140). Collections are not passed to the scalar guard. Populated override collections are accepted and skipped, rather than rejected.
6. `config_hash::from_parser/selected_copy` at `hash_table.cpp:134–143` uses the full Table model; insertion/growth and retained destination capacity/positional recursive assignment are implemented in this current path. The historical eight-key SelectedCopyOrder adapter is not selected by FilePass. No new missing algorithm was inferred from its old README.

`bootstrap-runtime-source/observed_runtime.cpp:9–11` has a dynamic type check only for fresh empty achievement paths. It does not detect CFG dialects or switch FilePass to the legacy reader. The actual image's installed provider flows through that same BootstrapFacade.

## What the legacy restriction actually controls

`configuration_factory::ScalarDocuments::read` (`factory.cpp:19–25`) itself uses the current `read_config2` and hash→Node conversion, then explicitly calls `config_pipeline::require_scalar_application`. The guard at `pipeline.cpp:25–32` rejects Content/Entitlements children and AchievementNames/AchievementTotals/SteamIdOverrides children. Its retained historical API is real and deliberately limited.

It is reached through `Configuration::load_document` (`factory.cpp:118`) only when a source caller explicitly invokes that old method or uses `configuration::initialize_configuration(State&,Dependencies&)`, whose local DocumentPass at `configuration.cpp:240–246` uses the legacy `configuration::load_and_apply` and load_document call at line233. That overload is different from the actual FilePass startup composition. There is no observed original command-line flag, root object spelling, collection size, override-file presence, missing Config2, or parse outcome selecting it.

Therefore a valid native Config2 containing9+ children, Content/Entitlements objects or achievement child tables is not a demonstrated production rejection: it goes through the full application/hash route. A non-Config2 root does not establish an old-format mode: native/source select Config2's empty logical value. Supplying a custom HOST provider or explicitly calling the historical two-argument API changes the source composition; its collection rejection is not by itself evidence of an original DLL input-domain gap.

## Retained native authority

- **01c870** EID `ev_78a1faedf06f56d422e6ecc197bbe90ce7f34af0e9b01420eef8b6b8827bf25e` obtains the executable path then calls019ad0 with0 and1 for base/override, before the shared post-configuration logic. These are pass selectors, not syntax/dialect flags.
- **019ad0** EID `ev_5ec5f52f67388ea3296af7964bac7ecdff2ed88218b0b532744a87ae797c72dd` constructs the base/override path, reads03b3f0, handles one BOM, invokes014310, and on its successful result looks up **Config2** at019d42 and copies it at019d52. Scalar/collection application follows within the same function. No separate legacy document-format or collection-rejecting branch was observed in this producer.
- **014310** EID `ev_99422feca877e9a1163ea80688e0a0aba6aa617bc8e7bf32c62a480005fdd3b8` accepts the same root Objects/separators/EOL grammar until input end and throws on an invalid ordinary exit; it is not an alternate decoder selector. The current parser preserves that object/key-value structure. The previously closed duplicate-key/parser-status questions were not reopened here.
- Existing `anadius-config-application-source/consumer-audit.md` already resolves the reused section-copy and native collection-list order: Content/Entitlements plus the achievement child tables. `anadius-config-startup-source/review.md` records wiring of the new loader into the same single post-configuration body. These existing proofs answer the candidate rather than requiring another ceremonial native session.

Whole retained pseudo SHA256 `26fe3e2e2c6fe6459d39d8f3a95738b2f83bb7d6b333b61249bdc00abd1d5acb`; assembly SHA256 `9b8c894f85b1ca0fd090a95993ecd5048e59a41ae2459e2bbdc80215e8027a42`, original target identity `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`.

## Source hashes

| Path relative to outputs | SHA256 |
|---|---|
|anadius-bootstrap-owner-source/owner.cpp|3e09b48ef634b7fd02ff7002659870f67eef3e5de09c25ff8a5e02a88ffcf887|
|anadius-config-startup-source/startup.cpp|de1bf59f726deade3bbcc6ec1ed6c26000638e0aaefd8c56ce02618c7e6f7e88|
|anadius-config-application-source/application.cpp|27e32ac1010aa4c169148f189016deb02a78c32d881f99134aed0b3a8ec0d2bb|
|anadius-config-hash-source/hash_table.cpp|15df336c45bd842333e5e56588d4c79a4e581c6ec239c488370926f13251934f|
|anadius-configuration-factory-source/factory.cpp|4a0d5d9abca820382a8efc50245e1748d31395edcfedfce78270219ccb8d0d28|
|anadius-config-pipeline-source/pipeline.cpp|018c61c3354ae541b7e4e7506b7fcfb12600689772b9e54e2d99122065c54378|
|anadius-config-source/configuration.cpp|a2a2fa7980ddeeaece163b705213bdc5adfff48ae149ec39a122850032e25f6c|
|anadius-resources-factory-source/configuration.cpp|4380c771eb81f66ff95ba67879b59710e622bd1a45fe1789ef18627f4736a7f4|
|anadius-bootstrap-runtime-source/observed_runtime.cpp|3e3897158c0020525d1a3265c51dda924ddb3ebb910148ae3cdad328b58d0a64|
|anadius-parser-source/parser.cpp|700479cfca81eae57f85d1fb8786426ffcc25d49edf9fa25359d00396252d2e2|

Recommendation: preserve the explicit legacy HOST guard as documented and do not treat its generalization as required original-DLL input recovery without a concrete native caller. Tracker classification may distinguish that optional interface limit from actual image behavior, but this audit changes no tracker and awards no full-DLL completion percentage. Other configuration details remain open only on their own evidence; this result does not close them generically.

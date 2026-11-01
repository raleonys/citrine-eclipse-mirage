# B8 — Embedded PNG and achievement persistence

ConfigurationPersistence binds exactly two existing configuration methods:
write_embedded_png_03c020 and update_achievement_storage_01eb00. Other nineteen
methods forward to its mandatory owned lower provider. Combined with B5/B7 and
the independent three-route filesystem wrapper, this supplies **18/21 selected
configuration routes**. Document fallback, logging and the separate license
subsystem remain explicit. These are source bindings, not new native bodies,
a complete replacement DLL, or a completion percentage for the original.

## Immutable embedded payloads

| Original VA | Length | SHA-256 |
|---|---:|---|
| 0x1800809d0 | 7488 | 833d5a6444b352bec759221e1973d7045b88967f4507059c6ffd95aa15cf34dd |
| 0x180082710 | 3882 | 9584cb4afe2734f4b30f0fe30d8390e76dd8703ac603fcf4a13c8f2df11133f5 |

Exact bytes came from retained read_bytes Evidence
ev_1585c277e2b5dffb5a54c26a1db122db985c13d72fea8b0b6272dbea69b9b082,
base0x18007f000/length138752 in outputs/anadius-rea-evidence.json, SHA-256
c6d1daab6e5cce17f48285d07f7b7a1f6c9be4b37f043f19694009393b401495.
Generation checked both PNG signatures, every chunk CRC32 and IEND. The parent
separately read eight bytes at0x1800a1108 through static Ghidra:
401d00002a0f0000 = little-endian7488/3882. Thus the extents also equal the
original caller length globals; padding was not guessed from IEND.
Length evidence:
ev_4ef179003503ecb3da7cac478ce0c307d43c59bd64f90052b3e6d3cfc189bf91.
The complete provenance is embedded-png-evidence.json. Byte arrays are in
embedded_png_data.hpp; embedded_png returns a stable read-only span and rejects
unknown enum identities.

Retained03c020 and existing write_binary_03c1d0 have the same application output
sequence: open mode0x20/share0x40, initial stream-state guard, one byte write,
explicit close. PNG output reuses that existing source. The bootstrap caller
ignores the boolean: failed initial open therefore performs no write and
returns without a new exception. Write/close exceptions still propagate.
No post-write integrity check, mkdir, retry or rollback is added.

## Owned storage graph

Constructor inputs are shared Filesystem, explicit initial achievement Paths,
owned lower configuration and PersistenceResources. Construction performs no
file I/O. Member order keeps resources/paths/effects/filesystem alive through
Storage and lower-provider destruction. The wrapper is immovable. paths()
returns a read-only view for the same quiescent controller lifetime.

Existing Storage.update_01eb00 supplies XML path construction/import,
count/progress/name/timestamp merge, optional Steam INI path and epoch setup,
status lookup and source INI writing. Concrete effects reuse existing
filesystem_widen_006d70, timestamp_to_epoch_041e10/epoch_041ff0,
status_01ea20, the same shared Filesystem.create_directories as the outer
wrapper, and existing binary file/INI source. No timezone or OS mktime
substitution is made.

Source failure prefixes remain: empty achievement set does not clear an
explicit old XML path; malformed XML does not invent successful records;
timestamp/status/mkdir errors can leave earlier paths or records changed.
The INI writer's boolean remains ignored as in the original caller.
Atomic replacement, backups, automatic retry and rollback are not added.

## Selected compatibility policies and limits

Default resources are NativeFactory, pinned xml_reader_bridge.Backend,
HostCrtNumbers, ClassicIso, NativeStats, NativeCalls and PortableErrors.
All are injectable through existing interfaces; null resources are rejected
before I/O. Compiler file sharing/streams/EH and MSVC object ABI are not proven
equivalent. Pinned XML source does not prove the original tinyxml2 release or
all malformed-input outcomes. HostCrtNumbers uses rebuilt host CRT; numeric
overflow and arbitrary locale equality remain unproven. ClassicIso is the
already recovered classic ASCII fixed-format parser and explicitly rejects
non-ASCII input. Its selection does not adopt an arbitrary OS locale.
Filesystem errors use selected compiler filesystem_error with operation, path
and preserved32-bit code, not original MSVC error-object/EH ABI.

Mutation/storage effects belong to the serialized controller. No background
writer, concurrent persistence consistency, cross-thread callback ownership,
loader-lock or DLL-unload proof is added. B3/B4 joins and B6/B7 domain lifetime
contracts still apply.

## Retained static evidence

Target SHA-256:
9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c.

| Address | Source/observation | Assembly Evidence ID |
|---|---|---|
| 0x18003c020 | raw span PNG output | ev_4dca6b0cf6f142a6ac6b4bd34a482777b9a683e3546558323a1b48efbf73b0b1 |
| 0x18003c1d0 | existing string output sequence | ev_97eaf0d1a5c0d1e7ad14855daa321d9f92e4aabb4a1d614bc467ef53f3010812 |
| 0x18001eb00 | XML import and INI setup | ev_bd10a5a82a8307f47db504ea903686ef7b68be9597de0c3a48f59707493363e8 |
| 0x18001f9a0 | existing Steam INI writer | ev_488d569fd12cfa9fe60d4d35d04b56c9d44b52396427f3060d14d2a7e0125a5a |
| 0x180041e10 | timestamp parse/error check | ev_c6b31cf960ca4a4bf60c21d21f3719c757a8a088e5e42c6c5b0a0784d2fc858d |
| 0x180041ff0 | arithmetic epoch conversion | ev_9d8c05e7881ace56f5053a11d8c9b6148320e73c05a8db0fd152a3ad78b0619c |
| 0x18001ea20 | status/error classification | ev_b2ed2f94d1e70182e9c643a28823161406f5467cb3c96081b5325e82fd10f8e8 |

Strict production compile passed with C++20/O2/Wall/Wextra/Werror/static.
Independent tests/common build are recorded by their authors and delivery
owner. This agent did not read original inputs or user CFG/token/achievement
content, execute a target or DLL, or modify prior source. Parent performed
only the declared static eight-byte length-global read of selected anadius.
No blocked version input or historical generated DLL was accessed.

# Pure-forward native audit — 2026-11-04

Fresh isolated REA session verified aligned server/package6.8.0, selected Ghidra12.1.4, and original `anadius64.dll` SHA-256 `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`. Collected22 fresh complete instruction/pseudocode replies across the six hooks and five helpers, including00aa50 cleanup. All succeeded; own session closed successfully.

| Typed callback | Pure predicate | Native original call |
| --- | --- | --- |
| RegOpenKeyExA | subkey==nullptr | 00f823 through a3840 |
| RegOpenKeyExW | subkey==nullptr | 00f983 through a3848 |
| CreateProcessA | application==nullptr | 00f4ac through a3868 |
| CreateProcessW | application==nullptr | 00f58c through a3870 |
| SHGetFolderPathA | exact32-bit id!=5 | 00fc53 through a3878 |
| SHGetFolderPathW | exact32-bit id!=5 | 00fcd3 through a3880 |

Null comparison helpers86b0/87f0 return1 at86e4/8824 before value lookup, conversion, or CRT comparison. Caller keys have SSO capacity15, so00aa50 skips heap free. Registry open calls original first, preserves its result, and cannot fabricate/insert on this null route. It still reads language-enabled bytea3ad1 atf8b7/fa17; direct source forwarding omits this inert read and localSSO/security bookkeeping.

Process null routing forwards all ten arguments unchanged. It requires no command-line inspection. Nonnull empty application is outside this predicate.

Folder helpers compare full ECX to5 at8dfb/8feb; every other exact integer bypasses Documents lookup and copy and leaves caller storage to original. Do not mask folder flag bits or broaden the id5 branch.

These observations support typed original-only source shortcuts. They establish no SDK-only nested phase gate. Earlier general-callback review step3 proposed that source policy explicitly; it is historical context and is not adopted here. Nested actor frames still need exact parent restoration, top-frame permission isolation, and retained typed provider/code ownership. This static evidence does not establish actual Windows reentry, general body concurrency, private CRT/PTD safety, or completed ABI.

Every current native result is a fresh MCP query, although content-addressed Evidence IDs match prior results when observations match. Earlier review/index files were read only as historical comparison and are distinctly recorded with hashes in `findings.json`. Complete fresh Evidence IDs are in `evidence-index.json`; session-before/session-active/open/close records retain identity and lifecycle replies.

No source edits, compilation, project tests, linking, target execution, binary mutation, or ZIP changes occurred.

# Active naming boundary

Documentation describing source provenance and historical evidence may retain
older names. Active source code, comments, tools, tests, and metadata must not
carry those names. New product schemas use the `surge.*` family. Vendored
source is preserved using byte pins; this cleanup does not change its contents.

Source QA reads the list below. Keeping the list in this document instead of
hard-coding the names in the checker makes the same naming boundary applicable
to the checker itself.

<!-- active-name-policy -->
```json
{"reserved_reference_labels": ["anadius", "yellowfield", "bluefield", "brownfield"]}
```

Document contents under `docs/` and `governance/`, as well as Markdown documents,
are excluded from this check. Root licensing/attribution documents named
LICENSE, COPYING, NOTICE, or AUTHORS are also preserved. Active file paths are
checked. The `build/`, `dist/`, and `.git/` directories and pinned
`third_party/` payloads are outside the active first-party scan.

Historical evidence now resides in the read-only archive under
`metadata/evidence/`. That directory is not treated as an active metadata
contract; its files are verified against historical identities and SHA-256
records. The naming check applies unchanged to new active source or metadata
files.

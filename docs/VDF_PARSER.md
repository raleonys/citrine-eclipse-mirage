# Existing Config2/VDF grammar with PEGTL

The owner's correction, translated as "the PEGTL VDF grammar already
existed," followed by authorization to continue, permits binding the
grammar already documented in RE to PEGTL. The earlier claim that the
grammar was unspecified was incomplete: the RE sources contain rules
and action ordering. Bluefield is read for comparison, not as an
application-code source.

The primary inputs are the owner-supplied `README.md`,
`evidence.json`, `parser.cpp`, and `parser_tests.cpp` under
`Reverse Engineering/anadius-parser-source/`.
Rule text for `vdf::parser::KeyValue`, `Object`, `Seps`,
and `tao::pegtl` in `anadius-all-functions.pseudo.c`
corroborates these inputs.

| RE routine | Rule / action |
|---|---|
| 014310 | Root: EOL, separator, or quoted object until EOF; reject mismatches |
| 013810, 013640, 0141a0 | Quoted byte string; decode only quote and backslash escapes |
| 013460, 0135b0 | Line comment and HT/LF/VT/FF/CR/SP whitespace |
| 013ed0, 013520, 013ab0 | Block comment; distinction between direct consumption and rewind |
| 0138e0, 013b70, 013cc0 | Scalar before object; members and mandatory scalar separator |
| 012ff0 | Create an object frame after reading its name, before validating opening `{` |
| 012cf0, 012bf0, 0164f0 | Complete assignment; replace duplicate subtree |

`include/preloader_l/config/vdf_grammar.hpp` defines the byte-level rules.
`src/config/parser.cpp` provides PEGTL actions/controls, the owned
`ParseError` interface, and current Node conversion.
`parser::parse` always invokes the real
`tao::pegtl::parse<vdf::Document,...>`. There is neither the former
hand-written parser nor an alternate backend after failure.

Scalar tokens are not accepted at the root; object names, keys, and
values are quoted. Scalar key/value pairs require a separator, while
completed members may be adjacent. A block comment ends at the first
`*/`. Unknown escapes, NUL, and invalid UTF-8 bytes are preserved;
raw CR/LF within a quoted token is an error. One UTF-8 BOM is removed.
LF counting is host-independent. Input rewind does not undo action
`State`; assignments and open/orphan frames before an error are retained.
Duplicate keys replace the entire preceding node, including values
and children. A fresh `State` is used per document and input must
remain alive throughout parsing.

On Windows, `preloader_l_vdf_objects` compiles against the
`preloader_l_pegtl` target. Its objects link into both the
`preloader_l_core` test archive and actual `preloader_l` DLL.
DocumentPipeline and the existing configuration application path use
this parser. Through the DLL, the dynamic source host checks a
BOM-prefixed nested Config2 document, duplicate assignments, and
error locations. The numeric fixture tests only the vendor API.

This binding does not select `surge.cfg` / `surge_override.cfg`
or establish their precedence. Production Asio/filesystem adapters,
automatic bootstrap, Consumer ABI, and native Windows/MSVC/game
acceptance remain separate tasks. TinyXML2 owns XML; this grammar
is not the XML/LSX parser. Vendored files and TinyXML2 depth 500
remain unchanged.

All 115 existing parser assertions were retained without modification.
The new `vdf_parser_regressions` test checks 23,417 inputs against
golden observations from the former RE-based Yellowfield parser:
4,724 accepted and 18,693 parse errors. Inputs cover short token
combinations, all 256 byte values in different contexts, truncation/
deletion/insertion/replacement of documents, and deterministically
generated documents. No real owner configuration files are read.

Observations include input, result/error class, byte/line/column,
complete AST, assignment order, open frames, accumulator/key/value,
and BOM state. Numeric values are included in FNV64 using fixed
little-endian encoding. This is a regression fingerprint, not proof
of cryptographic equivalence, original native DLL behavior, or
exception ABI parity.

- Baseline package: `yellowfield-vendor-compatible-source.zip`
- Package SHA-256: `c2a4d4622bb9cf03dd889d5bb9b0040e817d94a25fe2fcb44f4bcece52aa0767`
- Former `src/config/parser.cpp` SHA-256: `bc93d8badfb33c89b268470ee95b27384bb4a6b700842a4f4f476f2c2075d139`
- Expected full-state FNV64: `ea70f5d56d33f404`

The former implementation is not added back to the product tree.

## C25-F001 depth exhaustion mitigation candidate

Before invoking recursive PEGTL Object rules, a linear lexical preflight
rejects the 129th structural opening brace. The selected limit is 128
objects, including the root. Literal braces inside quoted byte strings,
escaped quotes, line comments and nonnested block comments are ignored.
On rejection, the existing grammar ParseError reports the offending
post-BOM byte, LF-counted line and column. No action State is modified.
PEGTL remains the only grammar/action parser for all accepted input.

This is a new compatibility restriction requiring owner review against
real product Config2 depths before merge. The portable regression test
does not demonstrate Windows native stack or Consumer/game acceptance.
The baseline check compiled the same test harness with the earlier
parser and configuration source as a separate executable. Normal
CTest compares the new PEGTL parser against the pinned observations.
Current raw results are in
[VDF_PARSER_STATUS.json](../metadata/evidence/VDF_PARSER_STATUS.json).

Current runtime binding:
[RUNTIME_BINDING.md](RUNTIME_BINDING.md). Earlier vendor/parser counts
here are historical snapshot evidence; new application tests are
tracked in [RUNTIME_CURRENT_STATUS.json](../metadata/RUNTIME_CURRENT_STATUS.json).

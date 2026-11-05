# Completion error routing and native prefixes — 2026-11-03

SOURCE_IMPLEMENTED_NOT_COMPILED_OR_EXECUTED.

The bounded025480 read /025a70 write error-path comparison found no new source
category or EOF suppression gap. Both native functions cache the signed code,
obtain the misc category, then load the incoming category. They suppress only
code2 when the category's identity field matches misc. Logging uses the cached
code and saved category object, prefix, virtual message, append, newline widening,
put, flush and string release. Error paths neither consume/rearm the read nor pop
the output queue. Existing frame_read.cpp/write_finish.cpp preserve that order.

Actual Graph installs the same Policy generic/system/misc providers into Profile
and Runtime. Profile wraps each category while retaining its identity_08 and
binds the same category table into both completion contexts. Policy supplies
distinct compiler category formatting. Existing test sources cover misc2 silence,
generic2 logging, signed cached codes and callback mutation ordering. They were
inspected only, not executed or revalidated against the pending sources.

The adjacent025720 accept error path has no EOF suppression. It logs every
nonzero error then reaches0270e0 accept rearming. Existing flow.cpp preserves
that distinction. Its default Policy prefix was incorrect:

| Native address | PE file offset | Observed NUL-terminated ASCII | Source result |
| --- | --- | --- | --- |
|0x1800845c8|0x82dc8|`Read error: `|Already correct|
|0x1800845d8|0x82dd8|`Write error: `|Already correct|
|0x1800845f8|0x82df8|`Exception: `|Replaced HOST default `Accept error: `|

Native025720 loads0845f8 at0x180025966 and calls006880 at0x180025974.
Policy::Log already maps that pointer to Options::accept_prefix. The production
Image/Worker/Graph path does not override these defaults. Only the default text
and its evidence comment changed; explicitly configured prefixes remain allowed.

REA binary_session confirmed6.8.0 at the configured path and Ghidra12.1.4,
Java25.0.5. The opened anadius64.dll SHA256 matched the established original.
Fresh Ghidra read_bytes returned all80 requested bytes at0845c8 with Evidence
ev_2d1f841d60b0889372e96a6bf81f2762df3ce9f4ee3faac7038447a377974dfe.
Independent read-only PE section mapping matched that complete range and decoded
each of the three C strings. Complete tool results and retained native function
slices are linked in completion-error-review.evidence.json. Session closed.

One already-pending production header changed. The total remains62 source/header
differences from the frozen1017-source full build. No project tests were added
or run for this literal correction. No build, link, DLL execution or ZIP update
occurred. Native ostream/locale, MSVC category/vtable/unwind ABI and concurrent
application behavior remain unvalidated. No new native normal body, ABI closure
or overall completion percentage is claimed.

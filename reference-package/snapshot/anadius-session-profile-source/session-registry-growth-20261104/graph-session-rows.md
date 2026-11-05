# Graph session rows — source implementation

Changed only `outputs/anadius-application-graph-source/graph.cpp`. Exact pre-edit bytes are retained as `graph.cpp.before.txt`. Root owns the accompanying Graph/Profile header comments, native Evidence and combined status records. This implementation used the fresh native accept/session/greeting findings supplied by root; no REA session was opened by this subagent.

The Graph's independent `rows(options.profile.sessions)` allocation now grows until an incoming nonzero-generation token's slot fits. Initial capacity remains configured. Growth doubles from the existing allocation (minimum8 for a growth), checks vector max_size/arithmetic first, and allocates default Row entries before reading/copying old rows. Existing Graph.call/admit(true) and Parent checks run after allocation and after coherent publication. WinLastError is preserved around this extra metadata allocation/free.

Only after revalidating admission and the old vector's size/data identity does the helper copy all existing token/raw-Logic rows and swap under metadata. Row copy assignment is statically required to be nonthrowing. No Row reference or iterator spans the compiler allocation. Allocation failure before this commit leaves the old rows authoritative. A later source-check failure can retain an enlarged coherent vector through existing Graph poison semantics; no reusable retry or native EH-unwind equivalence is claimed.

make_logic validates generation before growth, checks that the selected row is empty and newer, allocates Effects/dependencies and Logic, performs source postchecks, then reacquires the row by index and revalidates its generation/liveness before publication. The row token/raw pointer, Logic.registered flag and live count commit together under metadata. No persistent row reference survives Effects/Logic construction or a source checkpoint.

A private registered flag distinguishes a newly constructed Logic from a published row. Before registration its destructor does not retire an unrelated/empty row; the native-profile base retains its own existing checks. This allows post-allocation admission failure to unwind a never-published Logic instead of terminating merely because it was not yet in rows. Published ready Logic retirement retains the existing exact pointer, retained-prefix and live-count checks. All old row token generations and raw Logic object identities remain unchanged by vector relocation; JobCall, retirement and retained(token) already resolve by index under their existing authority/metadata rules.

This removes a source Graph capacity rejection and does not model a native fixed16-row array or introduce arbitrary concurrent registry mutation, deferred startup or an unload protocol. The native session allocation/ABI and timing behavior remain separate verification work.

Final graph.cpp SHA-256 before independent review: `f16b7a4dd9cbc8557792da98a38a0243471228f18f52e7d0f5b18a8a677dabed`.

No compilation, project test execution, linking, reconstructed DLL execution or packaging was performed. Test-source additions and independent review are owned by other agents.

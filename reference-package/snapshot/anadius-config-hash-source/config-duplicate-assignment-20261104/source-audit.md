# Config2 duplicate assignment: independent source audit

Candidate CLOSED: no production change is justified. This audit did not run REA; root supplied the fresh action/assignment/constructor findings and retains their complete Evidence in this directory. No compiler, project tests, linking, target execution or packaging occurred.

## Active route

The selected Configuration FilePass calls `config_application::read_config2`, which reads raw bytes, invokes parser::parse and passes the selected Config2 child to config_hash::selected_copy. This is the active mutable hash-table route, not the conservative legacy ScalarDocuments reader.

## Why current duplicate handling matches

`parser::assign` records the completed assignment name, then replaces an existing logical child in place with `old=std::move(node)`. Thus the parent's unique-key slot retains its first-insertion position, while the child's complete name/value/subtree/history is replaced. A new unique key is appended only once.

Root's fresh native observations distinguish MOVE from COPY: scalar action012cf0 and completed-object action012bf0 call0164f0. That helper moves both strings, clears the old destination child table through018c90, then swaps the fresh source table fields. It does not preserve the destination's previous child-table capacity. The additional014b10 constructor likewise moves strings and swaps the new empty table with source, resetting source to mask7/buckets8. It is not a copy constructor that would introduce reinsertion ordering.

`config_hash::from_parser` can therefore reconstruct each final child table from that child's final unique children. Duplicate assignments do not grow or reorder the parent's existing unique-key slot; replaced child-table history is intentionally discarded by native move assignment. `completed_assignments` retains action chronology for analysis/tests, but no extra replay of duplicate values is needed for this normal final-table reconstruction. Scalar-over-object clears old children; object-over-scalar receives the fresh object's empty/default scalar value, consistent with whole-node move replacement.

The retained-destination-capacity behavior in Table::assign_from and positional child reuse in Node::assign_from belong to the later configuration COPY path0162f0. Applying those copy rules during parser duplicate MOVE assignment would introduce a new discrepancy. No such change was made.

## Reviewed current source hashes

| Source | SHA-256 |
| --- | --- |
| outputs/anadius-parser-source/parser.cpp | 700479cfca81eae57f85d1fb8786426ffcc25d49edf9fa25359d00396252d2e2 |
| outputs/anadius-config-hash-source/hash_table.cpp | 15df336c45bd842333e5e56588d4c79a4e581c6ec239c488370926f13251934f |
| outputs/anadius-config-application-source/application.cpp | 27e32ac1010aa4c169148f189016deb02a78c32d881f99134aed0b3a8ec0d2bb |

This closes only the proposed duplicate-child capacity/history mismatch in the examined normal path. It does not establish native MSVC layout, allocator failure side effects, exception ABI or complete Config2/DLL equivalence. Existing frozen source/build proofs retain their historical meaning.

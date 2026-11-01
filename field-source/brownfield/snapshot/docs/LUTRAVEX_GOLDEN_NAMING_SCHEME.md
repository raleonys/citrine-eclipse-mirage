# LUTRAVEX Golden Naming Scheme

## Purpose

The LUTRAVEX Golden Naming Scheme is the permanent naming-governance contract for preloader_l. It replaces the earlier assumption that release, Golden, verifier, research and project names live in independent namespaces. They do not. All codename-like root tokens belong to one global historical namespace.

## Core model

A root token is assigned once. After assignment it is permanently reserved against unrelated reuse. The same root may reappear only through an explicit, provenance-preserving derived lineage.

This distinction is normative:

- **Unrelated reuse:** assigning an old root to a new unrelated release, project, controller or authority. **PROHIBITED.**
- **Derived lineage:** intentionally inheriting a historical root into a related Golden/protocol/verifier/authority name. **ALLOWED.**
- **Superseding authority lineage:** deliberately using a historical release root to name a later Golden authority that constrains, corrects or supersedes the historical build meaning. **ALLOWED and preserved.**

Therefore `URSYNEX Golden Finalizer`, `ARYRA Golden Layout`, `MUSTAVEX Golden Metadata`, `CORVEXON Golden Trail`, `FALCION Golden Cleaning`, and `LUTRAVEX Golden Naming Scheme` are not unrelated reuse. They are explicit provenance-preserving lineage.

## Rules

1. New release codenames are globally unique root tokens.
2. Historical roots are permanently reserved against unrelated reassignment.
3. Derived Golden/protocol/verifier/authority identities may inherit a historical root only when the lineage and intent are explicit.
4. Historical anomalies remain documented; they are never rewritten to make history look cleaner.
5. Prospectively, one semantic version has one canonical release codename and one canonical release codename maps to one semantic version.
6. Corrections and package revisions do not receive a second release codename.
7. Every new root is added to the CHANGELOG registry in the release that assigns it.
8. Candidate names are checked against the embedded CHANGELOG registry, current package/documentation tokens, and Library/file-name history before assignment.
9. Release codenames retain the project style: an original Latin animal root combined with a Bakugan-like synthetic suffix.
10. Functional descriptors may remain functional labels, but previously used descriptor roots remain reserved when they historically occupied the identity namespace.

## Intentional inherited authorities


### URSYNEX

Origin: historical 2.9.3 release identity. Derived authority: **URSYNEX Golden Finalizer**. URSYNEX is the sole bounded mutating post-link Golden authority and remains reserved against unrelated future identity.

### ARYRA

Origin: historical 2.6.0 release identity. Derived authority: **ARYRA Golden Layout**. This reuse was deliberate: the Golden authority constrains and supersedes the historical build/layout meaning. ARYRA remains reserved against any unrelated future identity.

### MUSTAVEX

Origin: historical 2.9.5 release identity. Derived authority: **MUSTAVEX Golden Metadata**. This is intentional authority lineage, not a naming collision. MUSTAVEX remains reserved against unrelated future identity.


### CORVEXON

Origin: canonical 3.1.1 release identity, with the separately retained 3.1.2 naming-only anomaly. Derived authority: **CORVEXON Golden Trail**. The derived authority is read-only final-binary acceptance and does not create a new release identity.

### FALCION

Origin: historical reserved project root. Derived authority: **FALCION Golden Cleaning**. It governs release-cleaning and package-boundary policy; flat packaging is a responsibility, not part of the canonical protocol name.

### LUTRAVEX

Origin: historical 3.0.9 release identity. Derived authority: **LUTRAVEX Golden Naming Scheme**. The naming scheme inherits LUTRAVEX intentionally as the governance authority for all future codename assignment.

## Historical corrections captured by 3.1.3 and carried through 3.1.9

The earlier 135-identifier audit was incomplete. Current package/Library verification proves at least four omitted release roots: **CUNYRA, FALCORYNTH, LYNXAVEX, MUSTELDRAX**. The global namespace also reserves verified internal codename-like roots used in historical release/research scope: **AQUORIN, AUTHORYX, FALVEX, FELNIX, LUPRAX, LYNXOR, MUSTEL, PARDYX, URSORYX, URSVEX, VULPAX**.

The corrected pre-3.1.3 historical reserved set is **150 roots**. `MARTAVEX` was assigned to 3.1.3 and became historical/reserved; the pre-3.1.4 set was therefore **151 roots**. `LEPORVEX` was assigned to 3.1.4 and became historical/reserved, producing the **152-root pre-3.1.5 historical set**. `MELAVEX` was assigned to 3.1.5 and became historical/reserved, producing the **153-root pre-3.1.6 historical set**. `VIVERRAVEX` was assigned to 3.1.6 and became historical/reserved, producing the **154-root pre-3.1.7 historical set**. `GENETAVEX` was assigned to the superseded 3.1.7 transition candidate and is now historical/reserved, producing the **155-root pre-3.1.8 historical set**. `CIVETAVEX` was assigned to 3.1.8 and is now historical/reserved, producing the **156-root pre-3.1.9 historical set**. `CASTORADRON` was assigned to 3.1.9 and is now historical/reserved, producing the **157-root pre-3.2.0 historical set**. `SCIURONYX` is assigned to 3.2.0 and becomes reserved immediately on assignment.

## Known anomalies retained as provenance

- ORBIT: reused across 1.8.13 and 2.1.1.
- VIGIL: reused across 1.10.0 and 2.0.0.
- ACCIPTRIX: reused across 2.7.9, 2.8.0 and 2.8.1.
- 1.8.10: LANCER and LYNX both exist historically.
- 2.2.4: PROFILE and SINKS both exist historically.
- 2.6.0: ARYRA and BOVYRA both exist historically.
- 2.6.1: CUNYRA and OVYRA both exist historically.
- 2.8.5: LYNXORYX and MERLORYX both exist historically.
- 3.1.2: CORVEXON exists as a superseded naming-only artifact; VULPHEXON is canonical.

## Machine mirror

`metadata/LUTRAVEX_GOLDEN_NAMING_REGISTRY.json` mirrors this policy and the embedded CHANGELOG root set. `scripts/check_lutravex_golden_naming.py` fails closed on drift, duplicate roots, missing lineage, current-name collision, or CHANGELOG/JSON registry disagreement.

## Semantic-release heading governance

CHANGELOG release-level H2 headings are unique per semantic version. Intermediate remediation history remains beneath its canonical semantic release and may not create additional version-level identities.

## Bakugan-style suffix-family pool

Release codenames combine an original Latin animal root with one suffix family from the project naming pool. The retained pool is:

`ADRON`, `AXION`, `DRAX`, `ORYNTH`, `ORYX`, `ONYX`, `HEON`, `YRA`, `ATRIX`, `EXIS`, `NEX`, `AVYX`, `VEXON`, `HEXON`, `AVEX`, `VEX`.

This list is embedded here so future assignments do not silently collapse back onto one visual pattern. The 3.1.5–3.1.8 line overused AVEX/VEX-like endings; 3.1.9 rotated to **ADRON** and 3.2.0 rotates to **ONYX**.

Prospective rule: the new semantic release suffix family must differ from each of the three immediately preceding semantic-release suffix families. Highly repeated AVEX/VEX/VEXON/HEXON families remain usable after cooldown; they are not banned. Historical names are never rewritten.

### 3.1.9 historical assignment

- Version: **3.1.9**
- Codename: **CASTORADRON**
- Animal root: **CASTOR** — Latin *Castor*, beaver genus root.
- Suffix family: **ADRON**.
- Status: historical/reserved after the 3.2.0 assignment.

### 3.2.0 assignment

- Version: **3.2.0**
- Codename: **SCIURONYX**
- Animal root: **SCIUR** — Latin *Sciurus*, squirrel genus root.
- Suffix family: **ONYX**.
- Collision check: absent from the 157-root historical reserved set, absent from the clean-parent package/document token set, and no exact retained Library filename collision was returned during candidate checking.
- Cooldown check: ONYX differs from the three immediately prior semantic-release suffix families ADRON / AVEX / AVEX.
- Approval: explicitly user-approved for 3.2.0.
- CASTORADRON moves to historical/reserved status.


### 3.2.5 assignment

- Version: **3.2.5**
- Codename: **SQUALERIS**
- Approval: explicitly approved in Library governance master revision **2026-10-07.8** as part of the current 3.2.x codename batch.
- RANQUEL moves to historical/reserved status.
- The machine registry is authoritative for collision/reservation enforcement.

## Preserved predecessor naming-policy record

The following block is retained verbatim for documentary continuity. Current 3.2.0 assignment rules above remain authoritative.

<!-- DOC_PRESERVE_BEGIN:HF22_V5_LUTRAVEX_NAMING -->
# LUTRAVEX Golden Naming Scheme

## Purpose

The LUTRAVEX Golden Naming Scheme is the permanent naming-governance contract for preloader_l. It replaces the earlier assumption that release, Golden, verifier, research and project names live in independent namespaces. They do not. All codename-like root tokens belong to one global historical namespace.

## Core model

A root token is assigned once. After assignment it is permanently reserved against unrelated reuse. The same root may reappear only through an explicit, provenance-preserving derived lineage.

This distinction is normative:

- **Unrelated reuse:** assigning an old root to a new unrelated release, project, controller or authority. **PROHIBITED.**
- **Derived lineage:** intentionally inheriting a historical root into a related Golden/protocol/verifier/authority name. **ALLOWED.**
- **Superseding authority lineage:** deliberately using a historical release root to name a later Golden authority that constrains, corrects or supersedes the historical build meaning. **ALLOWED and preserved.**

Therefore `URSYNEX Golden Finalizer`, `ARYRA Golden Layout`, `MUSTAVEX Golden Metadata`, `CORVEXON Golden Trail`, `FALCION Golden Cleaning`, and `LUTRAVEX Golden Naming Scheme` are not unrelated reuse. They are explicit provenance-preserving lineage.

## Rules

1. New release codenames are globally unique root tokens.
2. Historical roots are permanently reserved against unrelated reassignment.
3. Derived Golden/protocol/verifier/authority identities may inherit a historical root only when the lineage and intent are explicit.
4. Historical anomalies remain documented; they are never rewritten to make history look cleaner.
5. Prospectively, one semantic version has one canonical release codename and one canonical release codename maps to one semantic version.
6. Corrections and package revisions do not receive a second release codename.
7. Every new root is added to the CHANGELOG registry in the release that assigns it.
8. Candidate names are checked against the embedded CHANGELOG registry, current package/documentation tokens, and Library/file-name history before assignment.
9. Release codenames retain the project style: an original Latin animal root combined with a Bakugan-like synthetic suffix.
10. Functional descriptors may remain functional labels, but previously used descriptor roots remain reserved when they historically occupied the identity namespace.

## Intentional inherited authorities


### URSYNEX

Origin: historical 2.9.3 release identity. Derived authority: **URSYNEX Golden Finalizer**. URSYNEX is the sole bounded mutating post-link Golden authority and remains reserved against unrelated future identity.

### ARYRA

Origin: historical 2.6.0 release identity. Derived authority: **ARYRA Golden Layout**. This reuse was deliberate: the Golden authority constrains and supersedes the historical build/layout meaning. ARYRA remains reserved against any unrelated future identity.

### MUSTAVEX

Origin: historical 2.9.5 release identity. Derived authority: **MUSTAVEX Golden Metadata**. This is intentional authority lineage, not a naming collision. MUSTAVEX remains reserved against unrelated future identity.


### CORVEXON

Origin: canonical 3.1.1 release identity, with the separately retained 3.1.2 naming-only anomaly. Derived authority: **CORVEXON Golden Trail**. The derived authority is read-only final-binary acceptance and does not create a new release identity.

### FALCION

Origin: historical reserved project root. Derived authority: **FALCION Golden Cleaning**. It governs release-cleaning and package-boundary policy; flat packaging is a responsibility, not part of the canonical protocol name.

### LUTRAVEX

Origin: historical 3.0.9 release identity. Derived authority: **LUTRAVEX Golden Naming Scheme**. The naming scheme inherits LUTRAVEX intentionally as the governance authority for all future codename assignment.

## Historical corrections captured by 3.1.3 and carried through 3.1.9

The earlier 135-identifier audit was incomplete. Current package/Library verification proves at least four omitted release roots: **CUNYRA, FALCORYNTH, LYNXAVEX, MUSTELDRAX**. The global namespace also reserves verified internal codename-like roots used in historical release/research scope: **AQUORIN, AUTHORYX, FALVEX, FELNIX, LUPRAX, LYNXOR, MUSTEL, PARDYX, URSORYX, URSVEX, VULPAX**.

The corrected pre-3.1.3 historical reserved set is **150 roots**. `MARTAVEX` was assigned to 3.1.3 and became historical/reserved; the pre-3.1.4 set was therefore **151 roots**. `LEPORVEX` was assigned to 3.1.4 and became historical/reserved, producing the **152-root pre-3.1.5 historical set**. `MELAVEX` was assigned to 3.1.5 and became historical/reserved, producing the **153-root pre-3.1.6 historical set**. `VIVERRAVEX` was assigned to 3.1.6 and became historical/reserved, producing the **154-root pre-3.1.7 historical set**. `GENETAVEX` was assigned to the superseded 3.1.7 transition candidate and is now historical/reserved, producing the **155-root pre-3.1.8 historical set**. `CIVETAVEX` was assigned to 3.1.8 and is now historical/reserved, producing the **156-root pre-3.1.9 historical set**. `CASTORADRON` is assigned to 3.1.9 and becomes reserved immediately on assignment.

## Known anomalies retained as provenance

- ORBIT: reused across 1.8.13 and 2.1.1.
- VIGIL: reused across 1.10.0 and 2.0.0.
- ACCIPTRIX: reused across 2.7.9, 2.8.0 and 2.8.1.
- 1.8.10: LANCER and LYNX both exist historically.
- 2.2.4: PROFILE and SINKS both exist historically.
- 2.6.0: ARYRA and BOVYRA both exist historically.
- 2.6.1: CUNYRA and OVYRA both exist historically.
- 2.8.5: LYNXORYX and MERLORYX both exist historically.
- 3.1.2: CORVEXON exists as a superseded naming-only artifact; VULPHEXON is canonical.

## Machine mirror

`metadata/LUTRAVEX_GOLDEN_NAMING_REGISTRY.json` mirrors this policy and the embedded CHANGELOG root set. `scripts/check_lutravex_golden_naming.py` fails closed on drift, duplicate roots, missing lineage, current-name collision, or CHANGELOG/JSON registry disagreement.

## Semantic-release heading governance

CHANGELOG release-level H2 headings are unique per semantic version. Intermediate remediation history remains beneath its canonical semantic release and may not create additional version-level identities.

## Bakugan-style suffix-family pool

Release codenames combine an original Latin animal root with one suffix family from the project naming pool. The retained pool is:

`ADRON`, `AXION`, `DRAX`, `ORYNTH`, `ORYX`, `ONYX`, `HEON`, `YRA`, `ATRIX`, `EXIS`, `NEX`, `AVYX`, `VEXON`, `HEXON`, `AVEX`, `VEX`.

This list is embedded here so future assignments do not silently collapse back onto one visual pattern. The recent 3.1.5–3.1.8 line overused AVEX/VEX-like endings; 3.1.9 deliberately rotates to **ADRON**.

Prospective rule: the new semantic release suffix family must differ from each of the three immediately preceding semantic-release suffix families. Highly repeated AVEX/VEX/VEXON/HEXON families remain usable after cooldown; they are not banned. Historical names are never rewritten.

### 3.1.9 assignment

- Version: **3.1.9**
- Codename: **CASTORADRON**
- Animal root: **CASTOR** — Latin *Castor*, beaver genus root.
- Suffix family: **ADRON**.
- Collision check: no CASTORADRON release title collision was found in the retained Library title search; the root is absent from the 156-root historical reserved set.
- CIVETAVEX moves to historical/reserved status.
<!-- DOC_PRESERVE_END:HF22_V5_LUTRAVEX_NAMING -->

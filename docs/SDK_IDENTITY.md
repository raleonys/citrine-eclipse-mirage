# Fixed advertised SDK identity

The owner pins **10.6.2.31**, with no build suffix, until an explicit update
request. The initial supplied text was `10.6.2.31-football_22338885`; the latest
owner correction removes its build label from the advertised identity. The
consumer binary and its embedded version were not independently verified here.

The former `Emulator/SDK` setting supplied the `version` attribute of the opening
LSX `Event/Challenge`. It selected no compiler SDK, Windows SDK, third-party code
or runtime DLL implementation. It is a protocol field visible to the consumer,
so it should not be described as merely an invisible label or guaranteed to have
no client-side compatibility effect.

## Format and ownership

Use four numeric components and the existing dots-to-commas wire convention:

```xml
<Challenge version="10,6,2,31" build="release" key="..."/>
```

The former build suffix is documentary provenance only. Its omission follows the
owner's formatting correction; no independent consumer rejection test is claimed.

`core/protocol_identity.hpp` owns the fixed wire string. Application holds a const
string for the lifetime of its sessions and binds the existing MessageHandler
version reference to it. The mutable `configuration::State::sdk` field, both CFG
assignment paths and their now-unused normalization steps are removed. Old `SDK`
keys in the main or override CFG are ignored; they cannot change the greeting.
The low-level RE XML serializer still accepts a version argument for its source
API/tests; the production composition supplies only the fixed identity.

Do not automatically update this value from a library/toolchain upgrade, file
resource, provider response or client request. A new owner request must explicitly
change the pin and its independent regression expectation. The separately pinned
PE file/product metadata and external delivery labels are different identities.

Client `ChallengeResponse version="2"` and related message/cipher negotiation are
unchanged. That protocol selector is not the former `Emulator/SDK` setting.

## Validation

The real application test provides conflicting SDK strings in both `surge.cfg`
and `surge_override.cfg`, opens an actual LSX connection and requires the exact
quoted four-component version. A suffix would fail that check. Config tests also exercise
obsolete SDK values in scalar/hash and main/override routes, preserving other
configuration effects. Storage/startup checks retain their phase/state assertions
without testing the removed normalization feature. Session/message tests continue
to cover existing negotiation separately.

Current results: [SDK_NUMERIC_PIN.json](evidence/SDK_NUMERIC_PIN.json). The preceding
full-string run remains historical evidence in [SDK_PIN.json](evidence/SDK_PIN.json).
Cross-build/Wine evidence
does not certify acceptance of the new identity by the actual consumer or game.

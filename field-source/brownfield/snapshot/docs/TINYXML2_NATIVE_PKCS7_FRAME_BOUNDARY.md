# Candidate09 — Native test-host PKCS#7 to XML boundary

Status: **EXPERIMENTAL / NOT PUBLISHED**. Parent: exact Candidate08 ZIP SHA-256 `ed09fea4dcc91303c44173e520b7444dee657fbc261517422af3bed2a2fe9a98`.

## Source-linked failure evidence

Candidate08 Windows native log: both frames received and decrypted; response length=208; event raw decrypted length=224; `stage=event shape_failed`, `close_count=1`, `suffix_gap=2`, `nul_count=0`. The production `encode_response` pads plaintext to 16-byte AES blocks using PKCS#7 and `decode_payload` deliberately returns the entire padded decrypted block. The 224/2 measurements match an XML payload of 222 bytes with two padding bytes, but the exact values of the last two bytes were **not** captured in the user log, so the padding-value hypothesis remains subject to the Candidate09 real native run. This is a more specific and testable cause than a generic TinyXML2 parsing mismatch.

## Change

Test host only: verify complete PKCS#7 padding (`1..16`, exact repeated bytes, whole AES blocks, nonempty XML) on EACH decrypted response and event frame and pass only the resulting XML length to the existing pinned TinyXML2 DOM validator. No generic trimming, no skipping malformed padding or XML, no combined response/event acceptance. Test DLL and all production, public include, third_party, exports, resource sources unchanged from Candidate08.

Source/portable proof is not Windows/native execution proof. Re-run Windows Native Acceptance on Candidate09 exactly; do not attribute any earlier Windows PASS to Candidate09. Historical CRT `/NODEFAULTLIB /ENTRY:DllMain` comparator remains open; full eight-shard QA and game acceptance remain pending.

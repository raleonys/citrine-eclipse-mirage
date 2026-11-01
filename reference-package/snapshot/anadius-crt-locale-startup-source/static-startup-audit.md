# Current-locale başlangıç ve kapatma kanıt denetimi

Hedef SHA-256: `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`. Salt okunur yerel PE ve saklanan REA çıktısı incelendi; yeni REA oturumu açılmadı. Üretim kaynaklarına dokunulmadı.

## Ana bulgu

`0x180064468`, ilk current-locale bağlamasını yapan 17 baytlık leaf gövdedir. Saklanan Ghidra işlev listesinde yoktur; yerel PE baytları ve LLVM static disassembly ile doğrulanmıştır. Bu bulguya yeni REA EvidenceID verilmez.

- `0x180064468: LEA RAX,[0x1800a17e0]`
- `0x18006446f: MOV [0x1800a52e0],RAX`
- `0x180064476: MOV AL,1`
- `0x180064478: RET`

Ham baytlar: `488d0571d303004889056a0e0400b001c3`; dosya ofseti `0x63868`. `A52E0` başlangıçta `.data` sanal sıfır alanıdır. İlk locale sayacı görüntüde1 olduğu için bu doğrudan atama yerine `0709d8` çağrısı koymak aynı sayaç davranışını vermez.

## Başlatma sırası

1. 04ba30 CALL04b584; 04b584 calls CPU/VCRT then 064594 after VCRT AL success.
2. 064594 selects half-open pair table08aa60..08ab60 then tailcalls0704f4. Walker visits16 pairs in ascending order; only callback low AL byte determines success.
3. First callback064468 directly stores initial locale VA into global current-locale and returns AL1. No reference increment, lock or allocation in this body.
4. Second callback0644b0 initializes five cookie-derived encoded-cache setter groups; locale binding already occurred.
5. Lock initialization i4 and PTD initialization i7 follow; multibyte initialization is i10; onexit wrapper06447c is i15.
6. 065338 is pretermination callback07f730, not an initialize-pair entry. It creates separate lock and unlock DWORD tokens4 then calls065104.
7. 065104 reads lock index throughRDX at6511b, locks at6511d, then compares one global slotA52E0 to initialA17E0.
8. If different, CALL0709d8 at6514d uses existing updater; MOV[RBX],RAX at65152 writes returned pointer again after all updater effects.
9. Single cell loop ends atA52E8; unlock token through savedR9 is read only at6515b, followed by unlock6515d. No current lconv/ctype/mb_cur_max alias refresh in this wrapper.

## Kesin tablo ilişkileri

`064594` → `0704f4`, tablo `[08aa60,08ab60)`; 16 adet 16-bayt initialize/uninitialize çifti. Bütün sıfır olmayan girişlerde PE DIR64 kanıtı JSON’da kayıtlıdır.

| Sıra | Slot | Initialize | Uninitialize |
|---:|---|---|---|
| 0 | `0x18008aa60` | `0x180064468` | `0x0` |
| 1 | `0x18008aa70` | `0x1800644b0` | `0x0` |
| 2 | `0x18008aa80` | `0x180067538` | `0x18006755c` |
| 3 | `0x18008aa90` | `0x180006500` | `0x180006500` |
| 4 | `0x18008aaa0` | `0x1800625b0` | `0x180062614` |
| 5 | `0x18008aab0` | `0x1800704cc` | `0x1800704e8` |
| 6 | `0x18008aac0` | `0x0` | `0x1800644f0` |
| 7 | `0x18008aad0` | `0x180064e90` | `0x180064ecc` |
| 8 | `0x18008aae0` | `0x180067890` | `0x1800678cc` |
| 9 | `0x18008aaf0` | `0x18006f9bc` | `0x180006500` |
| 10 | `0x18008ab00` | `0x18006f5f8` | `0x0` |
| 11 | `0x18008ab10` | `0x0` | `0x180006500` |
| 12 | `0x18008ab20` | `0x0` | `0x180064538` |
| 13 | `0x18008ab30` | `0x0` | `0x1800644f8` |
| 14 | `0x18008ab40` | `0x180006500` | `0x1800644a0` |
| 15 | `0x18008ab50` | `0x18006447c` | `0x180006500` |

## 065104 normal akış ayrıntıları

Lock değeri `6511b` anında okunur, lock çağrısı `6511d` yapılır; global karşılaştırma bundan sonradır. Eşit değilse `6514d` üzerinden `0709d8` çağrılır; `65152` aynı hücreye RAX sonucunu yeniden yazar. Bu son yazma, updater içindeki etkilere rağmen korunmalıdır. Unlock değeri ancak `6515b` anında okunur, unlock çağrısı `6515d` yapılır. `065338` iki ayrı DWORD stack yuvasını4 yapar: o anki RSP+0x40 lock ve RSP+0x38 unlock token’ıdır.

`065104` assembly EvidenceID: `ev_c0bb591d054c51baae2bf8cbae8bbf908c43e85d699a0b730f9dc7f0aa91fd4a`; pseudo EvidenceID: `ev_b183b7650c91500655f4a95389def832d0a79d893a8f688009716a8b7698b7f2`. `065338` assembly EvidenceID: `ev_7ca19f79fe9886ba8cca9aa6403bb40ca05a148198b37710e12323f635d24689`.

## Mevcut kaynak kapsamı

- 0x180064468: not found in pre-existing production sources; new direct initializer candidate.
- 0x18006447c: two-call wrapper not found; core 0643f8 already recovered — `outputs/anadius-crt-onexit-source/onexit.cpp`.
- 0x1800644b0: already recovered as initialize_encoded_globals_0644b0 — `outputs/anadius-crt-cache-source/cache.cpp`.
- 0x180065104: normal-flow wrapper not found; called updater already recovered — `outputs/anadius-crt-locale-thread-source/locale_thread.cpp`.
- 0x180065338: wrapper not found; current startup bridge leaves callback binding external — `outputs/anadius-crt-startup-bridge-source/startup_bridge.cpp`.
- 0x180064594 / 0x1800704f4 / 0x180070588: generic walkers and outer wrapper already recovered — `outputs/anadius-crt-initializers-source/initializers.cpp`.
- 0x1800709d8: already recovered as update_locale_nolock_0709d8 — `outputs/anadius-crt-locale-thread-source/locale_thread.cpp`.
- 0x18004b584 / 0x18004ba30: already recovered outer startup — `outputs/anadius-crt-entry-source/entry.cpp`.
- 0x1800648fc: already recovered PTD construction — `outputs/anadius-crt-ptd-data-source/ptd_data.cpp`.

## Sınırlar ve açık noktalar

- 064468 is absent from retained Ghidra function inventories. Its17 exact bytes and static LLVM disassembly prove this normal leaf flow, but no new REA EvidenceID or original function-count attribution is fabricated. Root may obtain fresh function-boundary evidence.
- Only normal flow is covered; SEH finalization, exceptional unlock and register-high return bits are not reconstructed as a new policy. Boolean walkers consume AL only.
- Current-locale is PE virtual zero-fill before callbacks. The observed pair table provides intended static callback ordering, not a trace of an executed process.
- Raw preferred-image callback pointer values are metadata only. Rebuilt code must bind its own callbacks and live storage.
- Source-coverage statements use the pre-existing source snapshot before root new-module implementation; no claim about all unrelated startup callback implementations.
- 06447c and064468 have different roles. Calling updater0709d8 to implement initial064468 would incorrectly add a reference to the image counter(initial1).

Tam adresler, saklanan EvidenceID değerleri, seçilmiş assembly gövdeleri, doğrudan çağrı yerleri, dosya hash’leri ve PE relocation kanıtı `locale-startup-audit.json` içindedir.

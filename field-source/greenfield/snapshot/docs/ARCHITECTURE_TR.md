# Greenfield mimari şartnamesi GF00

## Bağlayıcı dış yüzey
- `DllMain @1`, Win32 `BOOL WINAPI (HINSTANCE,DWORD,LPVOID)`; implementation CRT startup davranışından bağımsız özel entrypoint kurmaz.
- `SURGE @2`, `void __cdecl()`; HF19 sürümündeki boş export davranışını yalnız şekil olarak izler.
- `DllShutdown @3`, `uint32_t __stdcall(uint32_t timeout_ms)`; `0` yalnız tamamen serbest GF00 owner durumunda dönülür.
- Windows x64, MSVC, x64 PE; x86, debug-export ekleri, diğer işletim sistemleri kapsam dışı.

## Katmanlar (tasarım, uygulanmış sayılmaz)
1. **win_bootstrap:** loader-lock-safe attach/detach, CRT startup, shutdown bütçesi ve module self-pin politikası.
2. **lifecycle:** explicit owner graph; `request_stop` / drain / physical cleanup / retry. Başarısız `FreeLibrary` sonrasında owner silinmez.
3. **transport:** Asio/IOCP; localhost endpoint ve auth scope; immutable parsed frame, bounded input, cancellation and timeout.
4. **protocol:** strict LSX grammar, XML limits, sender/id/method, stable encoding, invalid input fail-closed.
5. **services:** route registry with metadata, typed request/response and unsupported-route policy, no simulated success.
6. **state:** transactional Config2/VDF/store; immutable generations and read leases; rejected commits preserve old state.
7. **platform:** OS-owned handle provenance; hook adapters optional and gated behind explicit need and fail-safe teardown.
8. **QA/evidence:** unit/property, auth/owner negative tests, Windows native host, independent Consumer verification, reproducible manifests.

## Mimari kurallar
- Başlatma tek ownership graph'a sahiptir; global durum kullanıcı ve OS kaynaklarından ayrılır.
- Double-close, dropped FreeLibrary failure, ABA handle reuse, tracking allocation OOM, shutdown during callback için negatif testler zorunludur.
- Kopyalanan kaynak modül varsa ABI/architecture/lisans/commit/path/hash ve yerel değişiklik kaydı gereklidir.
- Tüm user-visible state değişimleri staging + commit sonrası yayımlanır; invalid input etkisiz kalır.
- Request parsing & response rendering saf fonksiyon olarak test edilebilir.
- Encrypted/encoded transport protokolünün tam uyumluluğu ayrıca native test gerektirir.
- Auth/e-communication işlevleri yalnız yetkili, belgelenmiş test ortamında doğrulanır; kimlik/entitlement uydurup başarı taklidi yapılmaz.

## İnşa aşamaları
GF00: export shape and inert lifecycle; GF01: owner graph, fail-once teardown; GF02: transport/frame loopback; GF03: config transaction/parser; GF04: bounded route engine; GF05: optional platform adapters; GF06: Windows/Consumer native acceptance. Bunlar yayın veya onaylanmış semantic version isimleri değildir.

## QA ve tanım gereği tamamlanmamışlar
- Build öncesi: portable unit and static-export guard; target ABI is MSVC-only, current environment is Linux.
- Windows: real MSVC /MT build/link, `dumpbin /exports`, dynamic LoadLibrary/getproc, FreeLibrary, CRT imports, x64 machine.
- Protokol: end-to-end real LSX and native Consumer acceptance; until then feature state NOT_IMPLEMENTED.
- Publication: ayrı kimlik ve explicit user yetkisiyle geçerli governance master üzerinde G0/G1/G2/G4, Final'da G3.

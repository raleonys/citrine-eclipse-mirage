# BF02 FIX11

- Integrate pristine HF19 gulrak for actual module-adjacent config/override paths.
- Preserve exact native spelling, optional config/defaults/profile and native file IO.
- Fix false-ready listener after unexpected Asio dispatch failure; stop explicitly.
- Separate targeted Session OOM recovery from arbitrary dispatcher/rearm failures.
- Add Unicode/cwd/path regressions and preserve failed-run evidence classification.

# BF02 FIX10

- Run Config2/VDF through pristine pinned PEGTL grammar/action/control adapters.
- Preserve byte syntax, duplicate replacement, partial action state and diagnostics.
- Add 8766 frozen full-state comparisons against sealed FIX09, including depth edges.
- Keep optional config, SURGE defaults, numeric identity, hash order and public ABI.
- Re-run portable/sanitizer/profile/Asio checks; Windows and actual DLL Golden NOT_RUN.

# BF02 FIX09

- Use pristine pinned Asio for the active async TCP transport; require Windows IOCP.
- Own session buffers and fixed handler/IO workers through stop and destruction.
- Contain accepted-client allocation/start failures and prove later-client recovery.
- Preserve config/profile, frame cap, client cap and per-connection FIFO behavior.
- Add focused real TCP/large-write/fault/cancel/lifecycle/cap tests; native NOT_RUN.

# BF02 FIX08

- Align used pristine MinHook/TinyXML2 bytes with exact HF19 upstream commits.
- Move existing successful-tree XML depth policy outside vendor; add self-closing cases.
- Record original Asio/IOCP, PEGTL/VDF and platform-specific filesystem evidence.
- Keep FIX07 working profile/config and separate transport/parser/reliability work.

# BF02 FIX07

- SURGE default display name selected by user; original numeric IDs retained.
- Replace synthetic sample with working default-equivalent config/profile.
- Add config path/key and online identity-change diagnostics without identity values.
- Add legacy CFG, corrected-config parity and full 14-field profile regressions.
- Parent no-config startup/profile USER_CONFIRMED; FIX07 Windows NOT_RUN.

# BF02 FIX06

- Bind PRODUCTION MSVC output to all five real Golden gates before publication.
- Isolate NMake production build and pin Golden resource/export/link policy.
- Add LOCAL_BUILD FALCION manifest/payload gate and failure-path regressions.
- Preserve runtime/config behavior; actual Windows and game acceptance NOT_RUN.

# BLUEFIELD kaynak gelişimi

## BF02 FIX03 — 25 Ekim 2026

- Kullanıcı DLL ve startup logu incelendi; dört başlangıç hook kurulum aralığında kesiliyor.
- Tek tek erken enable yerine açık queue API ve kilit bırakıldıktan sonra tek ApplyQueued.
- Resolve/create/queue/apply faz ve status kayıtları; UTC-ms/PID/TID/FIX03 module kimliği.
- Yeni stage-plan testi ve iki source-policy negatif vaka; native faz sırası kontrolü.
- Vendor pin'i, hook kapsamı ve protokol davranışı değiştirilmedi.
- Windows/game fix kabulü henüz doğrulanmadı.

## BF02 FIX02 — 25 Ekim 2026

- Eksik CryptQueryObject gerçek MinHook target/resolver/typed-trampoline olarak eklendi.
- File/null/suffix yol seçimi doğrudan kullanıcı input paketinden türetildi.
- Mevcut engine ve unload lease sahipliği kullanılır; internal callback slotu11.
- CRYPT32 gerçek link dependency, tam 11 argüman ve BOOL/LastError korunumu.
- Yeni portable Unicode/suffix testi, native crypt fixture'ı ve iki QA negatif vaka.
- Kullanıcının kaldırılmış hook kapsamı yeniden eklenmedi.
- MSVC/native/oyun kabulü henüz çalıştırılmadı.

## BF02 FIX01 — 25 Ekim 2026

- Kullanıcının MSVC 19.51 logundaki C3861/C2065: native hook owner artık
  IPv6 `sockaddr_in6` tipi için `<ws2tcpip.h>` başlığını doğrudan içerir.
- SHA-1 padding fill değeri `std::uint8_t{0}` oldu; C4244 dönüşüm uyarısı kaynağı giderildi.
- Windows sayısal CRT taramalarında aynı `%d`/`%x` formatlarıyla `sscanf_s`;
  non-MSVC portable yolu korunur. Genel uyarı susturma seçeneği eklenmedi.
- Windows environment adapter statik CRT kaynağını `_dupenv_s` ile okur ve
  kopyayı kendi ömründe tutar; C4996 getenv uyarısı kaynağı giderildi.
- Bu bir tam kaynak snapshot'ıdır; önceki BF02 paketi değiştirilmedi.
- Windows yeniden derlemesi bu ortamda çalıştırılmadı.

## Uygulama kaynak projesi — 25 Ekim 2026

- Kullanıcının tam, bağımsız DLL uygulama paketi talebi doğrultusunda proje kuruldu.
- Tek application state ve gerçek config/profil/online/dispatcher/persistence bağları.
- Gerçek sahipli TCP transport, per-session cipher, frame sınırları ve join edilen işler.
- DllMain/SURGE/DllShutdown ABI, CRT-aware bootstrap/self-pin ve unload ownership.
- Somut native registry/process/folder/connect callback etkileri ve pinned MinHook.
- Eksik avatar owner/size aktarımı restore; native connection counter ve başlangıç
  bekleme kilitleri ayrıldı. Native çalışma henüz doğrulanmadı.
- KeyForLicense gövdesi aynı paketin retained pseudo/ASM kanıtından uygulandı.
- Production/Debug/Native/CRT hedefleri, PE checker, Native/CRT/provider fixture'ları.
- Source shard/negative/manifest/ZIP ve exact-package clean-extract araçları.

## Reddedilen BF01

Yalnız Config2 bf_* API'si ve araştırma/reference arşivi içeriyordu. Kullanıcının
tam DLL uygulama isteğini karşılamadığı için kabul edilmiş uygulama parent'ı sayılmaz.
Önceki test sonuçları o dar dilime aittir; bu snapshot'a runtime PASS taşınmaz.

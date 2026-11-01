# B24 — Original SDK çağrılarında gerçek PTD kontrolü

Seçili Policy, callback_scope mevcutsa sahipli Boundary oluşturur. Bu hem B23 code-owner/mühürlü original sağlayıcısı hem raw original slotları için geçerlidir. Callback scope verilmemiş doğrudan legacy yol korunur. OriginalEffects sınıf düzeni değiştiği için tüm tüketicilerin yeniden derlenmesi gerekir.

## Üç kontrol noktası

On SDK yolu tek typed original_call üzerinden ilerler: **gerçek PTD kontrolü → slot/provider çözümü → gerçek PTD kontrolü → SDK çağrısı → gerçek PTD kontrolü**. Orta kontrol, virtual resolver’ın PTD’yi değiştirip normal dönmesi durumunda SDK’yı başlamadan durdurur. Resolver ve SDK en fazla birer kez çağrılır. Resolver exception atarsa da izin tekrar kontrol edilir; SDK çalıştırılmaz.

Geçerli postcheck, aynı resolver/SDK exception_ptr kimliğini yeniden fırlatır. Geçersiz postcondition önceliklidir. Argümanlar,32-bit dönüş bitleri, yazılmış output/in-out prefix ve WinLastError korunur; çıktı rollback’i, slot/FLS onarımı veya provider retry yoktur. Metadata kilidi gerçek SDK çağrısına taşınmaz. Bu kontrol tüm callback gövdesinden önceki bütün yan etkileri geri almayı vaat etmez.

## Test ve kanıt

**56 core + 28 integration = 84 yeni kontrol PASS.** Core, on SDK yolunun tamamında kesin pointer/scalar argümanları, bitcast sonuçlarını, üç permission probe’unu ve çıktı prefixlerini sınar. İlk precheck başarısızlığında resolver/SDK sayaçları sıfırdır. Resolver normal dönüp izni bozduğunda orta kontrol SDK’yı engeller; resolver exception ve SDK exception kimliği/error korunması ayrıca sınanır.

Gerçek FLS/heap/admission ile entegrasyon owned yolda G/R/G/SDK/G, raw yolda G/G/SDK/G sırasını tam olarak doğrular. G gerçek FLS probe, R resolver’dır; attach/leave okumaları bu ölçümden ayrılır. Gerçek own native Context/SDK shim, argüman/HRESULT/output/LastError ve normal PTD/native lease drain davranışını doğrular.

Own terminal çocuk201, geçerli SDK exception’ının aynı kimlikle, çıktı ve hata koduyla, PTD/native lease drain sonrasında Errors sınırına geldiğini doğrular.202SDKnull,203fresh snapshot değişimi,204resolver normal-null,205resolver throw-null ve206raw SDKnull senaryoları manual gerçek admission kullanır. Typed ihlal, active+poison/readyfalse, sonraki resolver/SDK çağrısının engellenmesi, yazılmış prefix/error ve code-owner retention doğrulandıktan sonra doğrudan TerminateProcess kullanılır. Bilinmeyen PTD native cleanup’ının başarıyla tamamlandığı iddia edilmez. Generic abort239 ve unexpected return230 başarı değildir.

source-evidence.json, B23’ten retained on callback ve host setup/cleanup VA/Evidence ID/line/hash kayıtlarını taşır. Yeni source checkpoint tasarımı ayrı etiketlidir. B22 REA6.5.0/Ghidra12.1.4 oturum kimliği retained kullanılır; B24’te target açılmadı, yeni original byte/decompile sorgusu yoktur.

## Açık kalan sınırlar

B18–B23 single-active PTD/admission, aynı Execution, shared stream, compiler locale/encoding ve code-owner/slot mühürleme sözleşmeleri korunur. Raw free/ABA, geçici değiştirip geri koyma, arbitrary provider/original reentry, gerçek executable provenance, explicit engine uninitialize/unload ve native CRT/EH/ABI/unload/PE eşdeğerliği kanıtlanmış değildir. Code lifetime token, bağımsız external quiescence yerine geçmez.

Original/rebuilt DLL/game çalıştırılmadı veya değiştirilmedi; user CFG/token/profile/achievement içeriği okunmadı, AV engelli öğeler yeniden denenmedi. Yeni native gövde/kapanış0;487mapped/491eligible/124normal claim/52tarihsel kapanış değişmez. Tam DLL hazırlığı BLOCKED; toplam DLL yüzdesi bu bileşimden türetilmez.

Sonraki B25, kritik bağlantılar içindeki kalan7online foreign sağlayıcıyı ele alır. Configuration/license/legacy document ve B10 review borcu açık kalır. İstenen sıra1→2→3→5→4; genel CRT kuyruğu bekler.

## Slot adresleri

| API | Callback VA | Original pointer VA |
|---|---|---|
| connect | `0x18000fd00` | `0x1800a3838` |
| RegOpenKeyExA | `0x18000f800` | `0x1800a3840` |
| RegOpenKeyExW | `0x18000f960` | `0x1800a3848` |
| RegCloseKey | `0x18000f5b0` | `0x1800a3850` |
| RegQueryValueExA | `0x18000fac0` | `0x1800a3858` |
| RegQueryValueExW | `0x18000fb60` | `0x1800a3860` |
| CreateProcessA | `0x18000f3f0` | `0x1800a3868` |
| CreateProcessW | `0x18000f4d0` | `0x1800a3870` |
| SHGetFolderPathA | `0x18000fc00` | `0x1800a3878` |
| SHGetFolderPathW | `0x18000fc80` | `0x1800a3880` |

## B24 ortak doğrulama

**56 çekirdek + 28 entegrasyon = 84 kontrol PASS.** Ortak build 242 nesne (238 C++ / 4 C), 279 normal test grubu ve 19 framework terminal senaryosu içerir. Önceki 814 kaynak aynı; 4 önceki dosya açık hash kaydıyla güncellendi; toplam 822 kaynak. Yeni native gövde ve kapanış 0; 487 adres, 491 karşılaştırılabilir derlenmiş etiket ve 52 native kapanış korunur. Bileşim ile raw-all defterleri ayrıdır. Gerçek alt sağlayıcılar, foreign ABI, image unload ve PE girişi açık kalır.

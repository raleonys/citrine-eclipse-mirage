# B17 — Native callback kabulü ve Runtime kapanışı

B16’nın tek denetleyici thread / callback reentry yok sözleşmesi, **seçilen native SDK girişlerinde** artık ilk mutable state veya buffer erişiminden önce uygulanır. Yeni `Controller`, yabancı thread, reentry ve durdurulmuş girişleri reddeder. Bu, genel host-thread desteği veya özgün native kilit kurtarımı değildir.

**40 çekirdek +33 entegrasyon =73 yeni kontrol PASS.** Core sayısına14 own terminal child dahildir. Bağımsız review kaynak/EXE hashlerini kontrol etti; testleri tekrar çalıştırmadı. Önceki785 kaynaktan781aynı;4 açık kaynak değişikliği ve5 yeni dosyayla790toplam. Ortak build sonuçları teslim adımında eklenir.

## Giriş sırası ve sahiplik

1. Native wrapper immutable Context’i binding Lease ile yaşatır.
2. Optional Admission.enter, callback lambda’sından önce çalışır. Foreign/reentrant/stopped giriş body/state/buffer’a ulaşmaz. Null Admission eski source yolunu korur.
3. Controller kilidi body, original, provider ve Execution wait boyunca tutulmaz. Kabul edilen kapsam tek controller’dır; diğer thread’ler sıraya alınıp başarıyla yürütülmez.
4. Başarı ve exception unwind’da Admission.leave, native binding Lease bırakılmadan önce tamamlanır. enter başarısızsa leave çağrılmaz. Incoming/outgoing LastError bookkeeping’den korunur.
5. Owner unique_ptr admission’ı Context’ten önce kurar; Binding/Context/effects bittikten sonra yıkar. Factory kopyaları aynı one-shot tüketim kaydını paylaşır.

Context layout ve Owner/factory source ABI değişti; **bütün tüketiciler yeniden derlenmelidir**. SDK callback imzaları aynı kaldı. Dört değişiklik `validation.json` ve `source-evidence.json` içinde before/after hashleriyle kayıtlıdır. Yeni interface/owner özgün DLL sınıfı veya virtual ABI olarak sunulmaz.

## Kapanış ve retry

B4 source stop/join→exit cleanup drain→ayrı external disabled-hooks/no-new-native-admission proof→Controller.stop→native in-flight check/Binding.close→registry.finish sırası kullanılır. Gate, worker join veya disabled hook kanıtı değildir. Cached entry pointer’lar ve image unload hâlâ dış lifecycle sözleşmesidir.

False/throw proof **gate’in önceki durumunu değiştirmez**. Seçilen testlerin ilk false/throw yollarında gate açık kalır ve controller callback’i çalışır. Önceki bir başarılı proof stop’u tamamlamış, fakat final native close in-flight nedenle bitmemişse, sonraki proof başarısızlığı gate’i yeniden açmaz. Stop kalıcıdır. Aktif callback içinden close denemesi gate’i kapatamaz; callback bitince retry başarılı olur.

Failed exit registration boş cleanup kuyruğu bırakabilir; hook cleanup bu yüzden uydurulmaz. Own testte fake engine açıkça kapatıldıktan sonra proof ve close retry yapılır. Daha önce tüketilen cleanup prefix’i tekrar çalışmaz.

## Test kanıtı

Core14 child: on SDK entry için stopped admission ve erişilemez sentinel buffer, foreign-thread connect, original’dan nested wide callback, controlled original exception unwind ve wrong leave. Child’lar yalnız bu projeden derlenen test EXE’leridir. Success/unwind sırası, LastError, lock-free-body metadata erişimi, active close, nullable legacy yol ve factory tüketimi ayrıca doğrulanır.

Integration gerçek yeniden oluşturulmuş B4 Runtime + B14 fake engine + B16 concrete Policy + aynı-domain ApplicationMemory kullanır. Publish-before-hook, own typed original, final Runtime map/original-slot kimliği, wide folder, fabricated W open/close, join timeout, false/throw proof, failed registration, retry ve unattached close test edildi. Worker/provider/engine/OS original’ları kontrollüdür; gerçek hook/registry/socket/process/MessageBox/foreign DLL veya original/rebuilt DLL/game çalıştırılmadı. Own heap/FLS ve thread metadata kullanımı bu testlerin sınırıdır.

`source-evidence.json` retained Ghidra VA/Evidence ID/line/hash kayıtlarına bağlanır. REA6.5.0/Ghidra12.1.4 erişimi doğrulandı; B17’de target açılmadı, yeni original byte/decompile sorgusu yapılmadı. Önceki static kayıtlar yeniden kullanıldı. Original dosyalar değiştirilmedi; gerçek kullanıcı CFG/token/profile/achievement içeriği okunmadı. AV engelli version öğeleri yeniden denenmedi.

## Açık kapsam

Guard yalnız seçilen native SDK wrapper’larını korur. Doğrudan recovered core/Effects çağrıları ve diğer worker/map/config/stream kullanıcıları dış serialization ister. B16 compiler CRT/locale/EH/secure-copy/streambuf ve memory domain sınırlamaları sürer. Foreign host-thread PTD, genel eşzamanlı callback başarısı ve gerçek hook runtime eşdeğerliği çözülmüş sayılmaz.

Yeni canonical native gövde/kapanış **0**;487mapped/491eligible/124normalclaim/52tarihsel kapanış korunur. Source composition ilerlemesinden toplam DLL yüzdesi türetilmez. Tam DLL hazırlığı **BLOCKED**.

**B18:** gerçek host callback thread/PTD modeli ve worker/callback ortak durum sahipliği.7online foreign çağrı,configuration/license/legacy document veB10review borcu açık. Öncelik1→2→3→5→4; genel CRT kuyruğu bekler.

## B17 ortak doğrulama

**40 çekirdek + 33 entegrasyon = 73 kontrol PASS.** Ortak build 235 nesne (231 C++ / 4 C), 265 normal test grubu ve 19 framework terminal senaryosu içerir. Önceki 781 kaynak aynı; 4 önceki dosya açık hash kaydıyla güncellendi; toplam 790 kaynak. Yeni native gövde ve kapanış 0; 487 adres, 491 karşılaştırılabilir derlenmiş etiket ve 52 native kapanış korunur. Bileşim ile raw-all defterleri ayrıdır. Gerçek alt sağlayıcılar, foreign ABI, image unload ve PE girişi açık kalır.

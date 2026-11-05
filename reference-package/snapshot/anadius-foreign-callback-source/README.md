<!-- PENDING-SOURCE-CHANGES -->
2026-11-05: Kullanıcının açık isteğiyle güncel tam ortak derleme ve testler geçti. Üretim DLL linki başarılı; DLL yüklenmedi/çalıştırılmadı. 296 nesne, 356 test grubu ve 19 framework terminal senaryosu doğrulandı. Güncel kaynakların son tam derlemeye göre bekleyen farkı 0. Bu sonuç tam davranış/loader/native ABI eşdeğerliği kanıtı değildir. Kanıt: `dll-critical-path/current-rebuild.evidence.json`. Önceki aşama metinleri tarihseldir; ZIP sonucu ayrı paket kaydındadır.
<!-- END-PENDING-SOURCE-CHANGES -->

# B18 — Ardışık foreign callback ve PTD sahipliği

Farklı host thread’lerinden **ardışık** callback’ler artık açık bir aynı-domain PTD scope içinde çalışabilir. Tek aktif callback kabul edilir; overlap, reentry ve setup/closing sırasında desteklenmeyen girişler erken reddedilir. Bu, genel eşzamanlı worker/callback veya özgün native CRT eşdeğerliği değildir.

**42 çekirdek +35 entegrasyon =77 yeni kontrol PASS.** Dokuz mevcut kaynak değişikliği before/after hashleriyle kayıtlı; önceki790 kaynaktan781’i aynı. Beş yeni dosyayla toplam795 kaynak. Bağımsız inceleme kaynak ve EXE hashlerini doğruladı; testleri yeniden çalıştırmadı.

## PTD protokolü

CallbackThreads controller’da Ready domain’den ProviderLease alır. Foreign thread phase/busy alanlarını okumaz. Mevcut FLS PTD borrowed’dır; sahiplenilmez veya temizlenmez. Null slot için yeni sıfır PTD aynı heap’te ayrılır, publication sonrası mevcut recovered constructor çalışır. Fresh scope yalnız errno/doserrno değişikliklerini kabul eder; diğer PTD mutasyonu poison/retention gerektirir. Aynı thread ve fiber kimliğiyle çıkış zorunludur.

Cleanup, denetlenen **sentinel detach → destroy once → observed raw free → FLS clear** aşamalarıdır. Bilinen detach/free/clear hatası sahipliği ve aşamayı retry için korur; tekrar destructor/refcount/free çalıştırılmaz. Raw root işlemleri errno lookup üzerinden replacement PTD yaratmaz. Bu raw allocation/failure stratejisi yeni source bridge politikasıdır; özgün calloc/new-handler/invalid-parameter ABI eşitliği iddia edilmez. Native Admission.leave temizliği bitiremezse başarıyla dönmez.

## Publication ve kapanış

Host setup begin foreign fazı kapatır. Context yayını ve hook kurulumu sırasında controller’ın senkron callback’leri çalışabilir; foreign girişler host setup tamamlanıp **ayrı caller ready proof** doğrulanana kadar kapalıdır. Proof config/map/original/server-port yazılarının tamamlandığını ve Execution/cache/worker/fallback/stream/provider kullanımının kabul edilen fazda güvenli olduğunu sağlamalıdır. B18 bu dış proof’u henüz somut worker senkronizasyonuyla değiştirmez.

Runtime stop-sources sonrası **admission pause + native drain**, worker join ve exit/hook cleanup’tan ÖNCE kontrol edilir. Aktif original çağrı varken close false döner; engine uninitialize olmaz. Pause yeni girişleri kapatır, mevcut body’nin policy/PTD izinlerini korur. Callback ve PTD cleanup bitince retry ilerler. Son disabled-hooks/no-new-native-entry quiescence kanıtı hâlâ ayrıca gerekir. Join timeout veya false proof sonrasında closing gate kendiliğinden yeniden açılmaz.

## Test ve kanıt

Own std::thread’lerden ardışık W open→query→close aynı Runtime handle/map/belleğini kullanır ve her dönüşte geçici PTD temizlenir. Borrowed identity, locale/multibyte dengesi, allocation/sentinel/detach/free/clear failure ve retry, poison/unknown outcome, gate permission, warming/overlap/reentry, gerçek yeniden oluşturulmuş B4 Runtime kapanışında tutulan original ve failed-registration yolları test edildi. Test worker işleri controlled controller üzerinde bir kez tamamlandıktan sonra foreign readiness açılır; gerçek eşzamanlı worker başarısı sayılmaz.

Core beş poison child’ında başarı kodu yalnız active+poison predicate sonrası own TerminateProcess ile üretilir; genel abort239’dur. Integration Rejected fault’ları160–162; raw free failure TLS marker’ı ve abort observer163; callback normal dönerse230. Böylece secondary FLS/fixture abort yanlış PASS veremez. `own-cleanup-debugger.txt`, eski own test EXE’sinin Domain.leave→abort teşhisidir; final EXE runtime kanıtı değildir. Final doğrulama yeni EXE/hash kayıtlarıdır.

`source-evidence.json` retained Ghidra VA/Evidence ID/line/hash bağlantılarını ve source tasarım ayrımını içerir. REA6.5.0/Ghidra12.1.4 doğrulandı; B18’de original target açılmadı veya yeni byte/decompile sorgusu yapılmadı. Orijinal/rebuilt target DLL/game çalıştırılmadı veya değiştirilmedi. Gerçek registry/socket/process/UI/foreign DLL ya da kullanıcı CFG/token/profile/achievement içeriği kullanılmadı. Own heap/FLS/thread metadata kullanıldı. AV engelli version öğeleri yeniden denenmedi.

## Sınırlar ve sonraki aşama

Kaynak API/vtable/Policy Resources layout değiştiği için bütün tüketiciler yeniden derlenmelidir. Default legacy B16/B17 yolları korunur; B18 scope, eski genel foreign-thread kısıtını yalnız açık admitted same-domain PTD alanında genişletir. Direct core/Effects ve dış state/stream kullanıcıları hâlâ ayrıca sınırlandırılmalıdır.

Yeni native gövde/kapanış **0**;487 mapped/491 eligible/124 normal claim/52 tarihsel kapanış korunur. Toplam DLL yüzdesi bu bileşim artışından türetilmez; tam DLL hazırlığı **BLOCKED**.

**B19:** gerçek worker/Execution ve ortak durum publication/provider sözleşmelerini somutlaştırma.7 online foreign çağrı, configuration/license/legacy document ve B10 review borcu açık. Öncelik1→2→3→5→4; genel CRT kuyruğu bekler.

## B18 ortak doğrulama

**42 çekirdek + 35 entegrasyon = 77 kontrol PASS.** Ortak build 236 nesne (232 C++ / 4 C), 267 normal test grubu ve 19 framework terminal senaryosu içerir. Önceki 781 kaynak aynı; 9 önceki dosya açık hash kaydıyla güncellendi; toplam 795 kaynak. Yeni native gövde ve kapanış 0; 487 adres, 491 karşılaştırılabilir derlenmiş etiket ve 52 native kapanış korunur. Bileşim ile raw-all defterleri ayrıdır. Gerçek alt sağlayıcılar, foreign ABI, image unload ve PE girişi açık kalır.

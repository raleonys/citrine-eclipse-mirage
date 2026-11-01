# B19 — Kaynak başlangıç publication ve Execution koruması

Başlangıçtaki **server, online, libraries ve host** yazıları artık aynı Runtime’ın Execution kimliğiyle atomik bir kayıt üzerinden yayımlanıyor. Aynı Execution nesnesinin flag/native mutex bookkeeping ve QPC/frequency-cache erişimleri kısa bir source gate ile korunuyor. Bu ek senkronizasyon yeni C++ bileşimidir; özgün DLL’nin eşzamanlılık veya CRT ABI eşitliği iddia edilmez.

**19 çekirdek +16 entegrasyon =35 yeni kontrol PASS.** Dört önceki kaynak dosyası before/after hashleriyle açıkça değişti;795önceki kaynaktan791’i aynı. Beş yeni dosyayla800kaynak. İki bağımsız inceleme, kaynak ve root tarafından çalıştırılan quick EXE/hash kayıtlarını değerlendirdi; reviewer yeniden çalıştırması iddia edilmez.

## Publication sözleşmesi

`bind_foreign`, Milestones ile foreign callback Domain’ini birlikte oluşturur; gerçek callback view Execution adresinin bağlanan yayıncıyla eşleşmesini kontrol eder. Üç flag hücresi farklı olmalı ve ledger yalnız bir Execution’a bağlanmalıdır. Önceden1olan flag, ilgisiz flag veya duplicate event yeni bir tamamlanma aşaması oluşturmaz. Yanlış issuer/invalid stage ledger’ı geçersiz kılar. Reset, başka Runtime’da reuse veya var olan observer/attachment üzerine yazma yoktur.

Worker’ın prefix yazıları ardından ilgili `finish_flag_036820` native kilidi bırakır, source gate bırakılır ve atomik Server/Online/Libraries kaydı oluşur. Host kaydı, `initialize_host` normal döndükten sonra ve attachment `host_setup_complete` çağrısından önce oluşur. `acq_rel` RMW zinciri ve all-bits acquire okuması, önceki plain prefix yazılarını controller’a yayımlar.

Bu kayıtlar **worker join, sunucu sağlığı veya hook kurulum başarısı değildir**. Server kaydı, async accept/event loop başlamadan önceki port yazısını kapsar; event loop çalışmaya devam edebilir. Online/library kayıtları thread/FLS exit’inden öncedir. Host normal dönüşü engine-init hatasından da gelebilir; test bunu doğrular. Foreign faz yalnız controller’ın açık `activate_foreign` çağrısıyla, dört kayıt VE kalan provider/stream/fallback proof başarılıysa açılır. Worker olayları aktivasyon çağırmaz.

## Execution sınırı

Flag/nativeowner/count işlemleri ile clock/cache ayrı kısa kritik bölümlerdedir. Polling Sleep ve observer sırasında gate tutulmaz. Ek gate bookkeeping’i Win32 LastError’ı korur; Platform operasyonunun gerçek etkisini silmez. Native Error5/6 ve unlock exception yollarında önceki native state mutasyonları korunur; otomatik unlock, rollback veya güvenli retry uydurulmaz.

Bütün eşzamanlı erişimler **aynı Execution** üzerinden geçmelidir. Aynı state’i paylaşan ikinci wrapper, raw lock/predicate helper’ı ve dışarıdan plain flag/cache erişimi bu garantinin dışındadır. Recursive source gate, native acquire/release/QPC provider’ının reentry’sini güvenli yapmaz. Desteklenen OS sınırları nonreentrant ve bilinen sonuçlu olmalı; unknown exception sonrasında state yeniden kullanılmamalıdır. Recovered MutexState layout’u0x50ve offsetleri korunur; C++ Execution ve Resources layout’u değiştiği için bütün tüketiciler yeniden derlenmelidir.

## Test ve kanıt

Own std::thread’lerle polling Sleep sırasında başka thread’in finish ilerlemesi,8thread/800clock çağrısında tek frequency-cache initialization, observer çalışırken başka Execution çağrısının ilerlemesi ve plain worker payloadlarının acquire publication sonrası görünmesi doğrulandı. Seed/alias/unrelated/duplicate/wrong-issuer ve hata prefixleri de test edildi.

Gerçek yeniden oluşturulmuş B4 Runtime entegrasyonunda own server worker `run_context` içinde beklerken diğer iki source worker kendi thread’lerinde tamamlanır. Kayıtlar tam olsa bile provider proof false iken giriş açılmaz; controller proof true olduğunda ardışık foreign folder/connect callback’i gerçek yayımlanan4660portunu kullanır. Sunucu worker’ı bu sırada join edilmemiştir. Engine-init failure örneğinde dört kayıt tamamlanır ama eksik hook/provider kanıtı aktivasyonu reddeder.

`source-evidence.json`, retained Ghidra VA/Evidence ID/line/hash kayıtlarını içerir:036740(clock),036820(flag finish),036880(wait),029ea0(server),035b60(online),035850(libraries),00e310(host) ve0102b0(cleanup). Yeni original byte/decompile sorgusu yoktur. B18 REA6.5.0/Ghidra12.1.4 session kaydı retained’dır; B19 source-only çalışmadır. Original veya rebuilt target DLL/game çalıştırılmadı/değiştirilmedi; kullanıcı CFG/token/profile/achievement içeriği okunmadı. Own EXE/thread/heap/FLS metadata kullanıldı; AV engelli version öğeleri yeniden denenmedi.

## Açık işler

Paylaşılan stream/fallback/provider state, dışarıdaki yazıcılar ve PTD sınırları için kalan caller proof zorunludur. B18’in single-active PTD scope, overlap/reentry reddi, cleanup-before-join native drain ve bağımsız son external quiescence kuralları korunur. Genel callback concurrency, gerçek hooks/unload/PE/CRT ABI ve tam DLL eşdeğerliği açık.7online foreign çağrı, configuration/license/legacy document ve B10 tarihsel review borcu da kapanmadı.

Yeni native gövde/kapanış **0**;487mapped/491eligible/124normal claim/52tarihsel kapanış korunur. Toplam DLL yüzdesi bu source bileşiminden türetilmez; tam DLL hazırlığı **BLOCKED**.

**B20:** callback’in kullandığı sağlayıcı/stream/fallback alanlarını ve dış yazıcıları envanterleyip sıradaki somut sahiplik sınırını bağlamak. Öncelik1→2→3→5→4; genel CRT kuyruğu bekler.

## B19 ortak doğrulama

**19 çekirdek + 16 entegrasyon = 35 kontrol PASS.** Ortak build 237 nesne (233 C++ / 4 C), 269 normal test grubu ve 19 framework terminal senaryosu içerir. Önceki 791 kaynak aynı; 4 önceki dosya açık hash kaydıyla güncellendi; toplam 800 kaynak. Yeni native gövde ve kapanış 0; 487 adres, 491 karşılaştırılabilir derlenmiş etiket ve 52 native kapanış korunur. Bileşim ile raw-all defterleri ayrıdır. Gerçek alt sağlayıcılar, foreign ABI, image unload ve PE girişi açık kalır.

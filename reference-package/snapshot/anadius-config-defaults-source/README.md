# B28 — Yapılandırmanın kanıtlı başlangıç değerleri ve uygulama boşlukları

`configuration_defaults::make_configuration_defaults_036d20()` mevcut configuration::State için **41 alanın tamamını**, 6 emulator ve 14 game özelliğini kanıtlı başlangıç değerleriyle döndürür. Örneğin mevcut caller-zero seed ile boş kalan kullanıcı, ülke/para ve Game/Language varsayılanları artık açıkça seçilebilen doğru başlangıç durumuna sahiptir. Fonksiyon her çağrıda bağımsız değer üretir; mevcut caller state'ini değiştirmez, gizli mutable global oluşturmaz. Kullanım:

```cpp
#include "defaults.hpp"
auto state = anadius::configuration_defaults::make_configuration_defaults_036d20();
// state'i mevcut configuration/startup/runtime akışına açıkça verin.
```

## Adres ve veri kanıtı

Global initializer `0x180001390`, `036d20(&DAT_1800a39e0)` çağırır ve ardından atexit için `LAB_18007e0c0` kaydeder. Constructor Evidence ID `ev_0afb3b90dc9b59c4a3af0186fd6c29a0737d49e946a887f4b80f3f9dab0ad3d8`; assembly ID `ev_9499218ae591f1b20f05c73a1d096cc0653227eb0b1b87bff8348d5859aadaa9`. `field-map.json` her State alanının global VA, constructor offset ve ASM yazmalarını; 20 map çiftini ve retained rdata byte kanıtını içerir. Tam retained constructor/initializer kayıtları ve hashes source-evidence.json ile bu klasördedir.

Username `a3bd8/+1f8`="anadius", persona `a3bf8/+218`="1144668899", user ID `a3c18/+238`="1000200030000"; bunlar kişisel dosyadan okunmuş değerler değildir, retained statik constructor sabitleridir. `37047 MOV dword [+258],10100`, use-last=0/fake-auth=1/pretend-connected=1 değerlerini kurar. `3708c MOV word [+2b0],SI` server_port=0; `37093` block-reruns=0 yazar. SDK="10,5,96,47512", US/USD ve subscription metinleri korunur.

Game DISPLAY_NAME değeri **Game**; yapılandırılmış Content kaydı için başka bir yordamın GAME fallback'iyle karıştırılmaz. EXPIRATION tam olarak "0000-00-00T00:00:00" metnidir; normalize edilmez. LANGUAGES 179 byte ve özgün sıradadır. Emulator ve game language hücreleri ayrıdır. Kaynak state{} ile başladığında sıfır/boş kalan her mevcut alanın ctor yazması doğrulanmıştır; tahmini alan eklenmedi.

## Gerçek uygulama ve test kapsamı

39 core kontrolü Native yazmalardan çıkarılan alan gruplarını, 20 map değerini ve iki bağımsız instance'ı sınar. Own EXE allocator, factory'nin gerçek string/map allocation noktalarını sırayla başarısız kılar; kısmi ve başarılı construction destruction sonunda tracked allocation kalmaz. Bu, seçili compiler'ın source lifetime testidir; native MSVC allocation sırası/SSO/EH eşitliği iddiası değildir.

24 entegrasyon kontrolü gerçek configuration::apply_config2, FilePass binary-reader/parser/hash, base+override startup orchestration ve mevcut MemoryPrinter/handler gövdelerini çalıştırır. GetProfile, GetSettings, GetAllGameInfo, GetGameInfo ve GetConnected gerçek XML yanıtlarında başlangıç değerlerini kullanır. Eksik scalar ayarlar native başlangıcı korur; açık override'lar uygulanır; invalid boolean fallback, SDK normalization, low16 port ve game/emulator language ayrımı korunur. İkinci eşzamanlı canlı state bağımsız kalır. Dosyalar/OS etkileri yalnız synthetic memory providers/recorders'dır; gerçek CFG/profile/token içeriği veya network kullanılmaz.

## CRT'ye dönmeden bulunan gerçek boşluklar

030000..038fff aralığındaki 61 canonical fonksiyonun 32'si definition mapped, 7'si module-record-only, 22'si eşlenmemiştir. Eşlenmemiş 22'nin 19'u container/string/mutex gibi kütüphane yardımcısıdır; genel CRT kuyruğuna alınmadı. Uygulama adayları 036d20 başlangıçları, 037f70 aggregate cleanup ve 038920 fatal log/abort'tur. Bu sayım yalnız belirtilen adres aralığını kapsar. `application-gap-audit.json/.md` bağımsız sınıflandırma ve çağrı ilişkilerini içerir.

Mesaj zincirinde mevcut 025c40 receive,0261d0 write,033d70 message ve032d00 dispatch gövdeleri zaten vardır; yeniden eksik sayılmaz. 39 registry adı/33 unique yanıt gövdesi mevcut: 31 canonical ve2 outside-canonical LAB etiketi. Bunlar 2076 fonksiyon paydasına yeni fonksiyon olarak eklenmez. Asıl sonraki eksik accepted-session **023000 constructor +023d20 socket move +025720 accepted start** ve tek sahipli ServerBackend/Transport/Scheduling bileşimidir. Ayrıntılı bağımsız message-chain-gap-audit.json/.md bu klasördedir.

## Kalan sınır ve ilerleme hesabı

Bu fonksiyon, 036d20'nin **modele dahil State alanlarının post-construction değer projeksiyonudur**. Enclosing MSVC aggregate'ın mutexleri, module/object/ready alanları, State'te temsil edilmeyen byte-string ve diğer storage alanları yeniden kurulmuş sayılmaz. 01390 global initializer, atexit kaydı ve native aggregate destruction kurulmadı. 07e0c0 cleanup etiketinin tam037f70 güzergâhı kanıtlanmış değildir. Gerçek PE entry, tüm native ABI/unload/concurrency, licensing02a830 ve B10 review borcu açık kalır.

REA binary_session bu aşamada yeniden doğrulandı: REA6.5.0; Ghidra12.1.4 available; açık hedef yok. İşlev analizi retained eşleşen pseudocode/ASM/rdata üzerinden yapılır; yeni target byte/decompile sorgusu yoktur. Özgün/rebuilt DLL/game çalıştırılmaz veya değiştirilmez; AV engelli version girdisi yeniden denenmez.

834 önceki tracked source değişmedi;4yeni source ile838toplam. Yeni native tam gövde/kapanış **0**;487mapped/491eligible/124normal claim/52tarihsel kapanış korunur.41/41 alanın semantik başlangıcının tamamlanması, tam native constructor veya tüm DLL yüzdesi değildir. Sonraki B29 accepted-session kurulumu/sahipliği; öncelik1→2→3→5→4; genel CRT bekler.

## B28 ortak doğrulama

**39 çekirdek + 24 entegrasyon = 63 kontrol PASS.** Ortak build 246 nesne (242 C++ / 4 C), 287 normal test grubu ve 19 framework terminal senaryosu içerir. Önceki 834 kaynak aynı; 0 önceki dosya açık hash kaydıyla güncellendi; toplam 838 kaynak. Yeni native gövde ve kapanış 0; 487 adres, 491 karşılaştırılabilir derlenmiş etiket ve 52 native kapanış korunur. Bileşim ile raw-all defterleri ayrıdır. Gerçek alt sağlayıcılar, foreign ABI, image unload ve PE girişi açık kalır.

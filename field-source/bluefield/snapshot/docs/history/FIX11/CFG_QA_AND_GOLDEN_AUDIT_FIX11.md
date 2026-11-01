# BF02 FIX05 — CFG, QA, kod kimliği ve Golden incelemesi

Bu çalışma somut namespace/derleme/diagnostic düzeltmesidir. Config olmadan
oyunun açılma sorununun kapandığını veya tam Golden kabulünü ilan etmez.

## Gönderilen paket ve CFG

`emu_SURGE_Madden NFL 27 1.0.140.41856.zip` içinde beş dosya vardır:
Madden27.exe, preloader_l.dll, preloader_l.srg, surge.cfg ve surge.png.
Yüklenen EXE/DLL çalıştırılmadı. CFG UTF-8 Config2 ağacıdır; INI/JSON değildir.
Özel üç profil değeri yeni test fixture'ına taşınmadı; sentetik değerlerle
değiştirildi. Oyun alanları ve 24 achievement kaydının yapısı korundu.

| CFG grubu | Yeni uygulamadaki mevcut durum |
|---|---|
| Game.Name/Version/ContentId | Uygulanıyor |
| Languages/Language/LanguageRegistryKey | Uygulanıyor; CLI dil önceliği sürüyor |
| User içindeki dokuz profil alanı | Uygulanıyor; bütün 14 alanlı XML yanıtı test edilir |
| AchievementsSet/24 AchievementNames | Uygulanıyor |
| SteamAppId/SteamIdPrefix | Uygulanıyor |
| Emulator.Online | HF19'da tanınır; adopted Anadius apply gövdesinde desteklenmez |
| Emulator.SDK | Bu CFG'de yoktur; input Anadius default'u korunur |

Önceki BLUEFIELD örneğindeki SDK=9.9.0.0 sentetikti. Bu yüklenen CFG'den veya
referans EXE'den gelmedi. Özgün input-target DLL'in constructor'ındaki
10,5,96,47512 doğrudan DLL byte/ASM kanıtıyla doğrulanmıştır.

Gönderilen DLL SHA-256 `099fde0b22aa91b9c8879397b35331d0d1ebb9c21b8958568a9108cdbbb1df44`;
input araştırma paketinin Anadius target'ı
`9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`.
Aynı binary değildirler. Bu tek başına tüm davranışlarının farklı olduğunu veya
EXE'nin çözümlemeyi bozduğunu kanıtlamaz; iki CFG lehçesinin sessizce eşdeğer
sayılmaması gerektiğini gösterir. Online'ın FakeAuth/PretendConnected/provider
authentication durumuna nasıl bağlanacağı burada tahmin edilmez.

## Somut düzeltmeler

- Üretim/test C++ root namespace'i preloader_l oldu. BLUEFIELD proje/paket/log
  etiketidir. CMake core/engine ve compiler/test selector kimlikleri de preloader_l
  ailesine alındı. Source policy eski namespace'i yeniden ekleyen mutasyonu reddeder.
- C4702 kaynağındaki ExitProcess sonrası std::abort kaldırıldı. /we4702 owned
  C++ hedeflerinde aynı uyarının tekrarını hata yapar; uyarı bastırılmaz.
- Emulator.Online artık sessizce kaybolmaz: değer/log kimliği yayınlamadan açık
  unsupported-setting diagnostic'i üretilir. Bu semantik desteğin eklenmesi değildir.
- BROWNFIELD yerel-build disiplininden compiler selector environment temizliği
  ve tek build kökü için exclusive lock sözleşmeleri uyarlandı.

## BROWNFIELD testlerinin yeniden kullanılması

HF19 arşivinde 256 test kaynak/giriş dosyası sayıldı. Bu sayı, yeni DLL için
çalıştırılmış 256 test veya 256 başarılı senaryo anlamına gelmez.

Field ownership, yanlış bölümdeki alias/dil anahtarı, nested leaf ve comment
grammar vakaları `test_brownfield_contracts.cpp` ile Application API'sine
uyarlandı. Kaynaktaki Country constructor varsayılanı korunduğu için HF19'un
config-loaded/empty-field beklentisi taşınmış sayılmaz. Online, route default'ları,
Auth8, image request şekli ve native release ABI için tam eşdeğerlik iddiası yoktur.

`test_supplied_config.cpp` gerçek CFG'nin anonimleştirilmiş şeklini kullanır.
Oyun alanları, 24 achievement, Steam mapping, tam profil yanıtı ve Online
diagnostic'i gerçek parser/Application/Service/router üzerinden kontrol edilir.

Golden PE araçları/policy/selftest dosyaları HF19'dan byte-identical alındı;
`metadata/BROWNFIELD_QA_ADOPTION.json` her kaynağı ve SHA'yı bağlar.
Bu araçlar tools/golden altında ayrı protocol-test bağlamındadır; application
C++'ı BROWNFIELD'dan alınmadı. Öz testler sentetik PE byte'larını işler; oyun veya
yeni native DLL çalıştırmaz. Yeni C++ testleri ile birlikte portable/CTest'e bağlandı.

## FIX05'te Golden durumu (tarihsel)

Önceki production driver yalnız check_pe.py çalıştırıp golden=NOT_RUN yazıyordu.
Tam Golden araçlarını yeni DLL production yoluna bağlama işi yapılmamıştı.
Bu bir uygulama eksiğidir; PE/ABI/static CRT PASS tam Golden kabulü değildir.

| Protokol | Bu işte yapılan | Yeni production DLL kabulü |
|---|---|---|
| URSYNEX | Özgün araç/policy ve sentetik öz test | NOT_RUN / production bağlantısı eksik |
| ARYRA | Özgün araç/policy ve sentetik yapısal testler | NOT_RUN / production bağlantısı eksik |
| MUSTAVEX | Özgün araç/policy ve sentetik metadata testleri | NOT_RUN / mevcut local-build resource profili bağlanmamış |
| CORVEXON | Özgün araç/policy ve sentetik ABI/checksum/import/cookie testleri | NOT_RUN / MSVC link-policy bağımlılığı bağlanmamış |
| FALCION | HF19 checker'ın source/package bağımlılıkları incelendi | NOT_IMPLEMENTED for this source layout |

Araç selftest PASS, yeni DLL için bu beş satırın PASS olması değildir. Driver
artık console'da da Golden pipeline NOT_RUN yazar. Canonical sıra MSVC ->
URSYNEX -> ARYRA -> MUSTAVEX -> CORVEXON -> FALCION olarak korunur; eksik zincir
başka bir denetleyiciye aynı ad verilerek kapatılmaz. HF19'un FALCION checker'ı
o repository'nin manifest/layout/doküman ve belirli regresyon dosyalarına bağlıdır.
Yeni source layout ve local/production metadata profiline bağlanmadan aynen
çalıştırılması yeni projenin kabulünü oluşturmaz.

## Açık bulgular

1. Emulator.Online davranış uyumluluğu EVIDENCE_BLOCKED; source warning eklendi.
2. Dosyasız gerçek oyun başlangıcı OPEN; güncel FIX04/05 runtime kanıtı gelmedi.
3. FIX05'te Golden production zinciri NOT_INTEGRATED idi; FIX06 bağlantıyı tamamlar.
   Yeni gerçek Windows DLL'in beş aşamalı kabulü henüz NOT_RUN.
4. BROWNFIELD test evreninin tümü aktarılmadı. Yeni 25 C++ grup + protocol tool
   selftest, o evrenin 8/8 kabulü olarak sunulmaz.
5. Licensing constructor/commit ve original online-provider ABI için önceki
   açık kapsam devam eder. Bu namespace/QA düzeltmesi onları kapatmaz.
6. Namespace ve C4702 source düzeltmesi doğrulanabilir; yeni Windows/MSVC
   compile/link ve native test sonucu burada NOT_RUN.
7. HF19 test_route_defaults vakalarındaki child-element QueryImage ve
   GrantAchievement şekilleri burada invalid LSX request ile reddediliyor;
   mevcut adopted handler'lar attribute alanları bekliyor. İki fark gerçek
   Application/Connection yolunda gözlenip kaydedildi. Consumer'ın hangi şekli
   gönderdiği burada gözlenmedi; protokol uyumluluğu CLOSED sayılmaz.

Bu çalışma resmi release veya CURRENT_ARTIFACT güncellemesi değildir.

## FIX06 güncellemesi

Yukarıdaki Golden tablosu FIX05 bulgusudur; FIX06 üretim girişine gerçek beş
kapı bağlanmıştır. `docs/GOLDEN_PIPELINE.md` güncel akışı açıklar. Önceki
NOT_INTEGRATED ve NOT_IMPLEMENTED durumları kaynak bağlantısı bakımından
kapanmıştır. Gerçek Windows/MSVC DLL kabulü NOT_RUN; diğer runtime bulguları
OPEN kalır. Selftest sonucu native kabul yerine geçirilmez.

## FIX07 güncel durum

Kullanıcı son sürümde surge.cfg olmadan başlangıç ve profilin çalıştığını bildirdi.
Önceki no-config OPEN bulgusu USER_CONFIRMED düzeyinde kapanır; exact DLL SHA ve
Golden receipt sağlanmadı. FIX07 varsayılan adı SURGE olur, sayısal ID'ler korunur.
Eski CFG'nin exact etkisi ve düzeltmenin 14 alanlı profile eşitliği yeni entegrasyon
testindedir. Kapsam 26 C++ + 3 tool grubudur. FIX07 native/Windows/DLL Golden
NOT_RUN; eski CFG ile tüketicide anadius görülmesinin exact nedeni açık kalır.
Ayrıntı PROFILE_CONFIG_FIX07.md içindedir.

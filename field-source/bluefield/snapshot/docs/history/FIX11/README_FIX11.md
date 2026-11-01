# BLUEFIELD — BF02 FIX11

Bağımsız C++20 / Windows x64 DLL uygulama kaynak projesi.
`NON_RELEASE_WORK` / `NOT_RELEASE`.

Pakette uygulama kodu, `preloader_l.dll` MSVC hedefi, config örneği,
Production/Debug/Native/CRT girişleri ve kaynak/paket doğrulaması birlikte bulunur.
Windows/MSVC ve oyun çalıştırması bu ortamda yapılmadı. FIX04 için kullanıcının
MSVC logu başarılıdır; FIX06 için native kabul devralınmaz.

## FIX11 gulrak path ve dispatch hatası

HF19-pinned pristine gulrak, modül yanı surge.cfg ve override yolu için aktiftir.
Native yazım normalleştirmeyle farklılaşırsa çalışan FIX10 yolu korunur; dosya
IO/error kodları ve config/profil varsayılanları değişmez. QA'da bulunan Asio
beklenmeyen dispatch hatası artık ready=false ve kapalı listener ile görünür
servis failure üretir; sahte ready bırakmaz. Unicode/cwd ve ayrılmış OOM testleri
vardır. Ayrıntı docs/FILESYSTEM_FIX11.md; Windows/gerçek Golden DLL NOT_RUN'dır.

## FIX10 Config2/PEGTL entegrasyonu

Config2/VDF artık HF19 ile aynı pristine pinned PEGTL üzerinde çalışır. Özgün
paketten uyarlanan byte grameri ve action kuralları korunur; BROWNFIELD application
parser'ı kopyalanmaz. 115 gramer assertion'ı ve FIX09'dan sabitlenen 8766 tam
state/error karşılaştırması yeni aktif backend'i sınar. Optional config, SURGE
profili ve sayısal kimlikler değişmez. Modül yanı filesystem yolu FIX11 ile bağlanmıştır.
Ayrıntı docs/CONFIG2_PEGTL_FIX10.md; native/gerçek DLL Golden kabulü NOT_RUN'dır.

## FIX09 Asio runtime entegrasyonu

Gerçek accept/read/write yolu artık pinned upstream Asio kullanır. Windows
IOCP-only source guard'ı vardır. Tek IO thread ve iki handler worker'ı session
socket/buffer/Connection ömrünü sahiplenir; eski client-başına thread yolu kaldırıldı.
Mevcut 32-client sınırı, optional config ve SURGE profili korunur. Allocation-failure
ve stop/drain testleri eklenmiştir. Windows IOCP/gerçek DLL kabulü NOT_RUN'dır;
PEGTL FIX10 ile bağlanmıştır; filesystem ayrı aşamadadır. Ayrıntı docs/ASIO_TRANSPORT_FIX09.md.

## FIX08 dependency hizalaması

Kullanıcı FIX07'nin düzgün çalıştığını doğruladı. FIX08, MinHook ve TinyXML2'yi
BROWNFIELD HF19'un exact pristine upstream pinleriyle eşitler. Uygulama profili
SURGE ve çalışan sayısal kimlikler korunur. XML başarılı-tree depth=100 politikası
vendor yaması yerine owned reader adapter'ındadır; malformed-deep parity iddiası
yoktur. Asio/IOCP ve PEGTL özgün input kanıtında vardır; BLUEFIELD'ın farklı backend
seçimleri aynı özgün mimarinin kurtarıldığı iddiası değildir. Ayrıntılar
`docs/DEPENDENCIES_FIX08.md` ve `metadata/DEPENDENCIES_FIX08.json` içindedir.

## FIX07 profil ve config

Kullanıcı son sürümün dosyasız açıldığını ve profil yüklediğini doğruladı. Bu,
önceki no-config OPEN bulgusunu kullanıcı kanıtı düzeyinde kapatır; FIX07 Windows
kabulü değildir. Config yokken varsayılan ad kullanıcı seçimiyle SURGE oldu.
Çalışan sayısal kimlikler korunur. surge.cfg.example tüm dokuz profil alanını
çalışan varsayılanlarla içerir; DLL yanında surge.cfg adıyla kullanılabilir.
Config yolları, seçilen anahtarlar ve online kimlik-değişimi artık loglanır;
kimlik değerleri loglanmaz. Ayrıntı: docs/PROFILE_CONFIG_FIX07.md.

## FIX06 bağlı Golden üretimi

Production girişi artık MSVC → URSYNEX → ARYRA → MUSTAVEX → CORVEXON → FALCION
sırasını gerçekten çalıştırır. Herhangi bir aşama başarısızsa yeni DLL dağıtıma
kopyalanmaz. MSVC ham çıktısı korunur; finalizer yalnız staged kopyayı değiştirir.
Production CMake kökü `build/production/msvc` ve generator NMake Makefiles'tır.
FALCION sealed source manifesti ve flat LOCAL_BUILD DLL payload'ını doğrular.
Ayrıntılar `docs/GOLDEN_PIPELINE.md` içindedir. Bu bağlantı oyun başlangıcındaki
açık bulguları kapatmaz ve Windows'ta yeni DLL'in Golden PASS aldığı iddiası değildir.

## FIX05 kod kimliği ve QA

C++ root namespace ve core/engine kimlikleri preloader_l ailesindedir. C4702'nin
ExitProcess sonrası gereksiz abort çağrısı kaldırıldı; /we4702 regresyonu hata yapar.
Yeni gönderilen Madden CFG'nin anonim fixture'ı ve HF19 field-ownership/nesting/
comment regresyonları Application API'sine bağlandı. HF19 Golden araçlarının
sentetik öz testleri de QA girişindedir. Bunlar yeni native DLL'in Golden kabulü
anlamına gelmez. FIX06 production zincirini ayrıca bağlar.
CFG'deki Emulator.Online mevcut input gövdesinde desteklenmez; artık açık log
uyarısı vardır. Başlangıç/profil/Online/Golden açıklarının tamamı
`docs/CFG_QA_AND_GOLDEN_AUDIT.md` içinde kayıtlıdır. Bu snapshot oyunun açılma
sorununun çözüldüğü iddiası değildir.

## FIX04 config ve profil başlangıcı

Önceki uygulama sahibi kaynak constructor durumunu kurmuyor ve surge.cfg'yi
zorunlu okuyordu. FIX04 input 036d20 constructor'ının profil, SDK, flag ve
6 emulator / 14 game attribute varsayılanını önce kurar; CLI, base ve override
sırasını korur. Username/PersonaId/UserId için sonradan eklenen zorunluluk kaldırıldı.
Dosyasız, boş, kısmi, yalnız override ve eksik Config2 kökü desteklenir.

Özgün input profil adı anadius idi; FIX07 ürün adı kullanıcı seçimiyle SURGE oldu.
1144668899 ve 1000200030000 sayısal kimlikleri korunur.
Geçerli config ve başarılı online sağlayıcı aynı source önceliğiyle bunları değiştirir.
Native profil cache'i özgün `%LOCALAPPDATA%/anadius/LSX emu` dizinini kullanır.
BLUEFIELD günlükleri ve diğer uygulama verileri önceki BLUEFIELD dizinindedir.
İki özgün 208x208 avatar geri getirildi; config ile seçilen avatar yoluna
dosya üretilmez. Kaynak cache koşulları korunur: UseLastOnlineProfile açık, sağlayıcı auth başarısız
ve cookie mevcut olduğunda kayıtlı üç alan okunur. Sağlayıcı yokken cache'i
koşulsuz yükleyen yeni bir davranış eklenmez. Önceki BLUEFIELD cache'i taşınmaz.

`tests/integration/test_startup_compat.cpp` profilin tüm 14 XML alanını ve settings/
game yanıtlarını sınar. Windows fixture'ı config yok/kısmi/override, online ve
başarısız auth sonrası geçerli/bozuk cache senaryolarını içerir. Windows fixture'ı
bu ortamda çalıştırılmadı. Bu düzeltme tüm uygulamanın veya oyun davranışının
kabul edildiği anlamına gelmez; ayrıntılar docs/BEHAVIOR_AUDIT.md içindedir.

## FIX03 başlangıç düzeltmesi

Kullanıcı logunda beş başlangıçtan biri ready, dördü MinHook initialization
sonrasında kesilmiş görünüyor. Kayıtta PID/zaman ve tam aşama olmadığı için
bunların aynı DLL/process ile ilişkisi ve kesin takılma nedeni henüz kanıtlı değil.
FIX03 mevcut kurulumdaki callback kilidi/per-hook erken aktivasyon riskini
kaldırır: tüm hedefler ve original slotlar hazırlanır, callback kilidi bırakılır,
tek `MH_ApplyQueued` çalışır ve yalnız başarıda commit ilan edilir.

Log artık UTC milisaniye, PID/TID, FIX03 module kimliği ve hook resolve/create/
queue/apply aşamalarını taşır. Oyun yeniden açılmazsa son girişin tam nerede
kesildiği görülebilir. Windows/native ve oyun sonucu hâlâ NOT_RUN durumundadır.

## FIX02 hook bağlantısı

`CryptQueryObject` gerçek CRYPT32 import adresinden mevcut MinHook motoruna
bağlıdır. Kaynak paketin file/null/suffix seçimi etkin yoldadır; diğer 10 argüman,
API çıktıları, BOOL sonucu ve LastError korunur. Hook sayısı 7 zorunlu + config
ile seçilen 4 koşullu hook'tur; iki koşul da açıksa 11 olur. Windows native
fixture dosya/blob/null/suffix yönlendirme ve unload sonrası çağrıyı da sınar.
Bu testin Windows çalıştırması ve oyunun açılışı henüz doğrulanmadı.

## Windows build

Visual Studio C++ x64 araçları, CMake ve Python 3 kurulu olmalı. Çalıştır:

```text
scripts\build_orchestration\build_production.cmd
```

Başarılı MSVC derleme, PE kontrolü ve beş Golden aşaması sonrasında çıktı:
`dist/production/preloader_l.dll`.
`surge.cfg` ve `surge_override.cfg` isteğe bağlıdır; DLL yanında aranırlar.
Dosya yoksa kaynak constructor varsayılanları korunur. Var olan bozuk veya
okunamayan dosya açık hata üretir. `surge.cfg.example` yalnız yorumlu ayar örneğidir;
örnek oyun veya profil kimliği yüklemez. Build mevcut config dosyasını değiştirmez.

Diğer girişler:

```text
scripts\build_orchestration\build_debug.cmd
scripts\build_orchestration\test_native.cmd
scripts\build_orchestration\test_crt.cmd
```

Native/CRT işlemleri ayrı build köklerinde test çalıştırır. Native test, yeni
DLL'i izole alt süreçte yükler; connect hook'u, LSX, ortam geri yükleme ve unload
sınırını sınar. İkinci test yalnız sentetik version-2 online sağlayıcı kullanır.

## Uygulama

DLL girişinin post-loader worker'ı config ve uygulama durumunu kurar, loopback
sunucusunu bağlar, native hook'ları kurar, profil/online başlangıcını tamamlar ve
istemcileri kabul eder. Mesajlar paketten alınan parser, AES, LSX dispatcher,
handler ve persistence koduyla işlenir. Üretim export'ları:
`DllMain @1`, `SURGE @2`, `DllShutdown @3`.

Kod atası `dll-reverse-engineering(6).zip` dosyasıdır. BROWNFIELD uygulama kodu
aktarılmadı; public ABI sözleşmesi ve onaylı operatör launcher sunumu esas alındı.
Köken ve uyarlamalar `metadata/IMPORTS.json`, kapsam ve sınırlamalar
`docs/ARCHITECTURE.md`, test durumu `metadata/ACCEPTANCE.json` içindedir.

Linux üzerinde uygulama çekirdeği ve gerçek TCP fixture'ı:

```text
python3 scripts/qa/test_portable.py
```

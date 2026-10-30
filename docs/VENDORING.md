# Kullanıcının vendor arşivleri

Kaynak, kullanıcının sağladığı dört aktif ZIP’tir. Asio için sonradan yüklenen
yeni arşiv, önceki girdinin yerini alır; diğer üç özgün arşiv korunur. Gulrak son kullanıcı talimatıyla çıkarılmıştır.
Brownfield vendor kopyaları ve internetten başka sürüm kullanılmaz.

| Vendor | Arşivde bildirilen sürüm | Yerleşim |
|---|---|---|
| Asio | 1.38.2; owner snapshot `bdf337ddb1a667c274372a602de5857ad3cb3ed0` | `third_party/asio` |
| MinHook | README'de v1.3.4 | `third_party/minhook` |
| PEGTL | 4.0.0 | `third_party/pegtl` |
| TinyXML2 | 11.0.0 | `third_party/tinyxml2` |

2835 dosya/link girdisi arşiv-member hash listeleriyle doğrulanır; lisanslar ve
Asio'nun `asio/include → ../include`, `asio/src → ../src` mantıksal bağlantı
hedefleri korunur. Windows'a taşınabilir kaynak ZIP'inde bu iki girdi, arşivdeki
hedef metnini aynı baytlarla içeren normal dosyalardır; symlink yetkisi gerekmez.
Derleme gerçek `third_party/asio/include` dizinini kullanır. Başlık/kaynak,
lisans, özgün ZIP ve vendor member hash'leri değiştirilmez.
Arşiv byte eşitliği, ayrıca doğrulanmış bir upstream Git commit iddiası değildir.
Input ZIP'leri kaynak paketine gömülmez; açılmış tam vendor ağaçları pakettedir.

```sh
python -B scripts/vendor_sync.py --root . --verify-installed
python -B scripts/vendor_sync.py --root . --archive-dir "/path/to/3RD PARTY" --verify-archives
```

Kurulum komutu `--install` dolu hedefin üstüne yazmaz. CMake yapılandırması
vendored kaynakları offline doğrular; upstream CMake, fetch veya paket yöneticisi
çalıştırmaz. Proje CMake hedefleri `preloader_l_asio`,
`preloader_l_minhook`, `preloader_l_pegtl`, `preloader_l_tinyxml2`.
Build sırasında da member hash kontrolü çalışır. Uyuşmazlıkta eksik/fazla/değişmiş
yollar ve beklenen/gerçek SHA256 yazılır; `1607/1607` yalnız toplamların eşit
olduğunu gösterir, dosyaların aynı olduğunu göstermez. Hash yenileyerek veya
denetimi kapatarak hatayı geçme. Standalone Asio hedefi,
Windows'ta var olan thread yeteneğini `ASIO_HAS_THREADS=1` ile bildirir;
LLVM-MinGW'nin `_MT` tanımlamaması yüzünden yanlışlıkla `null_thread` seçilmez.
Bu bir vendor kaynak patch'i veya thread testini devre dışı bırakma değildir.

## TinyXML2'nin 500 değeri

Eski RE kopyasında `TINYXML2_MAX_ELEMENT_DEPTH` 100'e değiştirilmişti.
Yeni arşivde **500**. Vendor varsayılanı korunur; kaynakta veya build ağacında
100'e döndüren patch yoktur. XML adapter testi artık bu gerçek vendor sınırını
doğrular: document seviyesi de sayıldığından 498 iç içe element kabul edilir,
499'da depth hatası oluşur. Bu, eski RE profile göre bilinçli olarak kaydedilen
bir davranış farkıdır; original/native eşdeğerlik iddiası yapılmaz.

## Derleme ve aktif kullanım ayrımı

MinHook ve TinyXML2 mevcut DLL hook-engine/XML hedeflerine bağlıdır.
PEGTL artık RE'de belgelenen mevcut Config2/VDF gramerini gerçek uygulama
parser'ında çalıştırır: [VDF_PARSER.md](VDF_PARSER.md). Ayrı sayı fixture'ı
vendor API kontrolü olarak kalır. Asio gerçek Application transport’una bağlıdır.
Windows storage/IO standard filesystem/fstream kullanır; Gulrak target’ı kaldırıldı.
Dört aktif kütüphanenin somut yolları [RUNTIME_BINDING.md](RUNTIME_BINDING.md) içindedir;
bu bağlantılar gerçek Windows/oyun kabulünün yerine geçmez.

## PEGTL ve derleyici uyumluluğu

PEGTL 4 public-input probe Linux/GCC'de çalışır. LLVM-MinGW **20251118,
Clang/libc++ 21.1.6, UCRT x64** ile de aynı public başlıklar derlenir.
LLVM-MinGW 20261006 / Clang 23'teki libc++ ise PEGTL'nin eriştiği private
`std::vector::__end_` alanını değiştirmiştir. Bu iki ayrı derleyici sonucudur;
vendor dosyalarına patch uygulanmaz, PEGTL sürümü değiştirilmez.

Varsayılan `PRELOADER_L_REQUIRE_VENDOR_COMPATIBILITY=ON` kontrolü,
`BUILD_TESTING=OFF` olsa bile PEGTL public API'si derlenmiyorsa CMake'i durdurur.
Önceki atlanan test, uyumlu Windows çapraz derlemesinde gerçek PEGTL çalıştırma
testiyle yer değiştirir. MSVC sonucu ayrıca doğrulanmalıdır; bu Linux oturumunda
MSVC çalıştırıldığı iddia edilmez.

```sh
export LLVM_MINGW_ROOT=/path/to/llvm-mingw-20251118-ucrt-ubuntu-22.04-x86_64
cmake --preset llvm-mingw-x64-vendors
cmake --build --preset vendors --parallel 3
```

Derleyici indirmesi vendor kaynağı değildir ve ürün ZIP'ine gömülmez.
Doğrulanan araç arşivinin SHA-256'sı:
`53a0c22caa46b501e1c089e1c31b24a7c0e0d5a86f8ad12b131aafd4cee01ef4`.
Asıl vendor pinleri değişmez. Güncel sonuçlar ve araç kökeni
[VENDORING_STATUS.json](../metadata/evidence/VENDORING_STATUS.json) içindedir.

Yalnız eski/uyumsuz derleyiciyi teşhis etmek için ayrı build dizininde
`-DPRELOADER_L_REQUIRE_VENDOR_COMPATIBILITY=OFF` kullanılabilir. Bu mod yalnız vendor teşhisi içindir. PEGTL testi **SKIPPED/UNAVAILABLE** kalır;
gerçek VDF parser hedefi bu derleyicide yine derlenemez. Uyumluluk PASS sayılmaz.
Derleyici tanısı yeni CMake'de `CMakeFiles/CMakeConfigureLog.yaml`, eski CMake'de
`CMakeFiles/CMakeError.log` içinde bulunur. İlk Clang 23 hatası
`metadata/evidence/VENDOR_PEGTL_COMPILER_DIAGNOSTIC.log` içinde korunur.

`vendor_io_contracts`, Asio ile gerçek loopback TCP accept/connect/read/write,
EOF ve iptal completion’larını; standart filesystem ile Unicode dizin/dosya,
binary içerik, yeniden adlandırma, listeleme, silme ve eksik dosya hatasını sınar.
Bağlantı yalnız kernel'in seçtiği geçici loopback portuna açılır; geçici dosyalar
testin kendi dizininde temizlenir. Bunlar uygulamanın üretim transport/storage
politikasını veya nihai bootstrap'ını tanımlamaz.

## Windows'ta ZIP açma

Güncel kaynak ZIP'i düz köklüdür ve yalnız normal dosya girdileri taşır.
7-Zip, WinRAR veya Windows'un standart ZIP açma işlemi kullanılabilir.
Yeni paketi temiz bir klasöre aç; önceki bozulmuş açılımın üstüne birleştirme.
İsteğe bağlı doğrulamalı açma aracı:

```sh
python -B scripts/extract_source.py ../yellowfield-windows-vendor-source.zip ../yellowfield-clean
```

Araç CRC/hash/member/type kontrolü yapar ve mevcut hedefin üstüne yazmaz.
Yeni paket için Developer Mode, yönetici yetkisi veya `restore_vendor_links.py`
gerekmez. Yalnız VENDOR_LOCK'taki iki yolun birebir hedef metni kabul edilir;
boş/yanlış metin, yanlış symlink, dizin kopyası veya başka symlink reddedilir.
Gerçek symlink içeren eski ZIP'ler tarihsel paketlerdir; eski açma koşulları
onlar için geçerlidir. `restore_vendor_links.py` yalnız isteğe bağlı olarak
doğru metni gerçek bağlantıya dönüştüren araçtır; derlemenin önkoşulu değildir.

Güncel runtime bağı: [RUNTIME_BINDING.md](RUNTIME_BINDING.md). Buradaki önceki vendor/parser test sayıları tarihsel snapshot kanıtıdır; yeni app testleri [RUNTIME_CURRENT_STATUS.json](../metadata/RUNTIME_CURRENT_STATUS.json) içindedir.

Gulrak kaldırma yetkisi ve Windows IO geri dönüşü: [WINDOWS_IO_FIX.md](WINDOWS_IO_FIX.md).
Asio Windows tüketicileri Brownfield gibi _WIN32_WINNT/WINVER0x0601 alır.

Güncel commit kanıtı ve yalnız build kopyasına uygulanan, hash ile sabitlenmiş
SAFE-FREEZE istisnası: [HOOK_ENGINE_HARDENING.md](HOOK_ENGINE_HARDENING.md).
Orijinal dört vendor ağacının üye hash'leri değişmez.

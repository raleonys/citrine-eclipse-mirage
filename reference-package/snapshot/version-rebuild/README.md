# version.dll dış katmanının kısmi yeniden yazımı

Bu proje, incelenen `version.dll` dosyasının **dış katmanını** C++ ve x64 MASM kaynaklarıyla yeniden kurar. Özgün C++ kaynak kodu kurtarılmış değildir. 197 dışa aktarılan sarmalayıcı ve dış `DllMain` yeniden yazılmıştır; içerideki **21.504 baytlık PE bileşeni özgün makine kodu olarak korunur**. Bu nedenle “tüm DLL C++ kaynağına dönüştürüldü” denemez. `anadius64.dll` bu projenin kapsamına girmez.

Proje, resmi LLVM-MinGW 20261006 UCRT x64 paketindeki Clang 23.1.3, llvm-ml ve llvm-rc ile başarıyla derlendi. Derlenen PE'de 198 export/ordinal, 7 import ve 197 sarmalayıcıdaki 6.556 komut statik olarak karşılaştırıldı. 424 dal hedefi, 636 RIP üzerinden bellek başvurusu ve gömülü 21.504 bayt eşleşti. Sürüm kaynağının 560 baytlık payload'ı da özgün kaynakla SHA-256 düzeyinde aynen eşleşti. Çalışma zamanı eşdeğerliği henüz doğrulanmamıştır. Hiçbir DLL yüklenmedi/çalıştırılmadı ve özgün dosya değiştirilmedi.

## İçerik

- `loader.cpp`: dış girişin C++ rekonstrüksiyonu.
- `wrappers.asm`: 197 sarmalayıcının tutulan assembly kanıtından mekanik çevirisi; 6.556 özgün komut.
- `version.def`: 198 ad ve ordinal; `DllMain @3` dahil.
- `version.rc`: özgün `RT_VERSION` tür16, ad1, dil1033 kaynağının metinsel yeniden yazımı. Fixed-file-info, altı sürüm metni ve dil/codepage çevirisi korunur; tüm derleme yollarına dahildir.
- `embedded_original_machine_code.inc`: **C++'a çevrilmemiş özgün PE makine kodu**; sabit bayt dizisine dahil edilir.
- `export-evidence.json`: her sarmalayıcının özgün adresi, ordinal'i, modülü, önbellek yuvası ve iki kanıt kimliği.
- `validation.json`: sayımlar, gömülü bileşenin SHA-256 değeri ve sınırlar.
- `generate.ps1`: üst dizindeki `version-reconstruction-input.json` kanıtından üretilen dosyaları yeniden oluşturur.
- `validate-static.ps1`: üretilen assembly'yi ters sembol dönüşümüyle özgün 6.556 komuta tek tek karşılaştırır; export ad/ordinal'lerini ve tüm gömülü baytları denetler. Bu, C++ yükleyicinin derlenmiş davranışını test etmez.
- `build-llvm.ps1`: mevcut LLVM-MinGW araç zincirini kullanarak CRT olmadan derler; araç kaynağını/sürümünü/hash'lerini ve tam komutları `build-llvm/build-record.json` içine kaydeder.
- `verify-compiled.ps1`: derlenen DLL'i yüklemeden PE tablolarını ve llvm-objdump çıktısını inceler; linker map'teki özgün adres etiketlerini kullanarak tüm sarmalayıcı komutlarını, operand genişliklerini ve kontrol akışı hedeflerini karşılaştırır. Sonuçlar üst dizindeki `version-build-verification` klasöründedir.

## Gözlenen davranış ve kanıt

| Bileşen | Özgün konum | Gözlem |
|---|---|---|
| WinMM sarmalayıcıları | örnek `auxGetDevCapsA 0x180001000` | 180 işlev `0x18000e078` modülünden her çağrıda adla çözümlenir. |
| Dil adları | `0x180004d2c`, `0x180004d83` | `VerLanguageNameA/W`, `0x18000e080` kernel32 tanıtıcısından her çağrıda çözümlenir. |
| Version sarmalayıcıları | `0x180004dda..0x180005468` | 15 işlev `0x18000e088` modülünden çözülür ve `0x18000e000..0x18000e070` yuvalarında önbelleğe alınır. |
| Dış giriş | `0x180005469` | Yalnız `DLL_PROCESS_ATTACH` için sistem DLL'lerini açar, gömülü PE'yi belleğe eşler ve `mappedBase+0x3338` girişini çağırır; her durumda 1 döndürür. |
| Gömülü PE | dış VA `0x180006d80` | 21.504 bayt; tercih edilen taban `0x180000000`, görüntü boyutu `0xa000`, 5 bölüm, import RVA `0x5c1c`, relocation RVA `0x9000`. |

Ana kanıtlar:

- Dış `DllMain` dossier: `ev_dfd20118a4db8cf9932db18a1fc7452d57fcbee254114f5d52c1988eaa8b92f5`.
- PE başlığı/export tablosu: `ev_13b4368c7e6bd415810e14d335b39a540c563ec7d53fe5591ce2fbb27672d41e`.
- Gömülü PE baytları / `.rdata`: `ev_157c446e2c1bce1a022bed8f576a7cc535fa15c41a6478fc2ef751d2902f97a5`.
- `auxGetDevCapsA` pseudocode: `ev_dcf8e03b22dcd469585b9a641efc6ac979f944c282014e985ad5338040bb790f`.
- Gömülü bileşen SHA-256: `36e775ed97b7a41c4f0cb98b94ee8edb74fab5cfe17d5fa8cfed265228301846`.
- Sürüm kaynağı: `ev_f8081bd25ceeaf6de9090e01f1b9c4f3991b000a05038b0823ef7b8f60b3ed93`; özgün payload dosya ofseti `0xd058`, 560 bayt, SHA-256 `d61413c8dfa84eb63f499c0f3b5bc63e198538db28e9a793d86ac64403799f26`. Derlenen kaynağın aynı hash'e sahip olması, bütün sabit alan/metin değerlerinin ve sıralarının bayt düzeyinde korunduğunu doğrular.

Sarmalayıcıların başarısızlık dönüşleri, 32 bit yazmalarla yapılan sıfır genişletmeleri, özgün register saklama/geri yüklemeleri, yığın argümanları, `JMP RAX` kuyruk çağrıları ve önbellek davranışı assembly üzerinden korunmuştur. Yalnız sembolik adresler yeni yerleşime uyarlanır. Kaynakta eklenen MASM unwind yönergeleri yeni derlemenin unwind metadatasını üretir; bu metadata özgün dosyayla bayt eşdeğeri olduğu iddiası taşımaz.

C++ yükleyici sabit ofsetleri ve gözlenen kontrol akışını izler. Özgün kodda görülmeyen TLS çağrısı, unwind kaydı, detach/serbest bırakma veya hata kurtarma davranışı eklenmez. Derleyicinin komut seçimi ve yeni dosyanın PE yerleşimi farklı olacaktır. `DllMain` ve sarmalayıcıların yeni RVA'ları özgün RVA'larla eşleşmez; export ad ve ordinal sözleşmesi korunur. Gömülü bileşen kaynaklarına ait açık kalan çalışma, tam kaynak kurtarımının tamamlanmadığı anlamına gelir.

## Derleme

Bu ortamda kullanılan taşınabilir araç zinciriyle PowerShell komutu:

```powershell
.\build-llvm.ps1 -ToolchainPath '..\..\work\toolchain\extracted\llvm-mingw-20261006-ucrt-x86_64' -SourceArchivePath '..\..\work\toolchain\llvm-mingw-20261006-ucrt-x86_64.zip'
.\verify-compiled.ps1
```

Resmi dağıtım kaynağı: `https://github.com/mstorsjo/llvm-mingw/releases/download/20261006/llvm-mingw-20261006-ucrt-x86_64.zip`. Arşiv SHA-256 değeri, GitHub asset digest'i ile eşleşti: `317492c456aa27ee607a5919f1d2d38dcdc1112516a24d0bf4b00d078f52d17a`. Betik verilen arşivin bu hash ile eşleşmesini araçları çalıştırmadan önce denetler. Sistem kurulumu veya PATH değişikliği gerekmez.

LLVM'in MASM okuyucusu `option casemap:none` ve `.const` kısaltmasını desteklemediği için betik yalnız bu yönergeleri eşdeğer açık salt okunur `.rdata SEGMENT READONLY` bölüm tanımına uyarlar. Komut ve operandlar değiştirilmez. Asıl `wrappers.asm` MSVC biçiminde kalır.

Derlenen dosya `build-llvm/version.dll`; SHA-256: `c982e3ea13f0f8123899d4ab6175fe93925885f10a5c79417a39cecdcfbb5f7e`.

Statik karşılaştırma adreslerin yeniden eşlenmesine, RIP adreslerinin çözülmesine, `JE/JZ` ve `JNE/JNZ` eşanlamlılarına izin verir. Register'lar, genişlikler, sabitler ve aynı işlev içindeki dal hedefleri aynen denetlenir. Bu sonuç C++ `DllMain` için komut düzeyinde eşdeğerlik veya tüm DLL için çalışma zamanı eşdeğerliği kanıtı değildir. Gömülü PE hâlâ özgün makine kodudur.

Mevcut bir Visual Studio kurulumu ve Windows SDK içeren **x64 Native Tools Command Prompt** içinde bu dizine geçin:

```bat
build-msvc.cmd
```

Bu komut yalnız derler/linkler; DLL'i yüklemez. Çıktı `build\version.dll` olur. Alternatif CMake:

```bat
cmake -S . -B build-cmake -A x64
cmake --build build-cmake --config Release
```

Derlemeden sonra export/import incelemesi için salt okunur araç kullanılabilir:

```bat
dumpbin /exports build\version.dll
dumpbin /imports build\version.dll
```

Statik kaynak kontrolü PowerShell ile:

```powershell
.\validate-static.ps1
```

Hiçbir komut hedef DLL'i çalıştırmayı gerektirmez. Yeniden üretilen DLL'in kullanıma alınması ve çalışma zamanı karşılaştırması bu çalışmada yapılmamıştır.

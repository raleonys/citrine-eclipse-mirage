# anadius64.dll ek kitaplık yükleme katmanı

`libraries.cpp` altı adresli işlevin gözlenen kontrol akışını C++20 olarak yeniden kurar. `native_system.cpp`, gerçek Win32 çağrıları için derlenmiş bağlayıcıdır; bu incelemede çağrılmadı. Kaynak, bağımsız anadius64.dll üretmez. Kayıp günlük/UI, CRT exception ve başlangıç bayrakları `Effects` içinde zorunlu sözleşmelerdir. Yükleme kararları ve liste işlemleri bu sözleşmelere gizlenmiş değildir; gerçek gövdeleri kaynakta bulunur.

Özgün DLL SHA-256: `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`. Kanıtlar üst dizindeki `anadius-all-functions.asm.txt` ve `.pseudo.c` dosyalarında korunur. Yeni C++ kaynakları analist rekonstrüksiyonudur; REA tarafından üretilen özgün kaynak değildir.

| Özgün VA | Kaynak işlev | Assembly Evidence |
|---|---|---|
| `0x180035260` | `load_list_035260` | `ev_6f84080b037476cc604a9f5729174d06bcfd848f98adc03a20c0047535be64bf` |
| `0x180035850` | `load_phase_035850` | `ev_470b3dc481c525d0c42f0bd10877f4d8c1660c8c0851d2c8f073e021f1add0bf` |
| `0x180035ab0` | `unload_035ab0` | `ev_aae177604e1f3b5eae66baf7aea4442d18256389dda8675d68800483ea44a394` |
| `0x1800450a0` | `suppress_errors_0450a0` | `ev_9a546f3826f323b9e15c5954c944e3051bbfe1114897967b4c21eb6b0b188df0` |
| `0x1800450d0` | `load_copy_0450d0` | `ev_65e0e4834d2c1c0ea7b6859e2e77c457e679e94556ac682b9c9c05ab9c74540e` |
| `0x180048b30` | `convert_wide_048b30` | `ev_9261beaef45db73e50b6eca26833fe4fbc6e4a09863942eba65958c4fd04f7bc` |

`utf8_path` yeni ortak yardımcıdır; özgün dosyada bu isimde ayrı işlev yoktur. `0x18000a140` pseudocode/assembly, `0x1800450d0` içindeki kopyanın path normalizasyonu değil wide-string kopyası olduğunu gösterir. Kopya kaynakta `std::wstring` ile ifade edilir; MSVC allocator/SSO yerleşimi aynen üretilmez.

## Korunan davranış

Listenin boş olması dahil önce `GetErrorMode`, sonra eski değer OR1 ile `SetErrorMode` çağrılır. Önceki değer, `SetErrorMode` dönüşünden değil `GetErrorMode` sonucundan alınır. Her yol için günlük başlangıcı, boyut belirleyen UTF-8 dönüşümü, yazan dönüşüm, günlük bitişi, wide-string kopyası ve `LoadLibraryW` sırası korunur. Yol kanonikleştirilmez ve önek eklenmez.

Başarılı handle sonuç listesine eklenir. Başarısız yükleme listeye sıfır koymaz; yolu **ikinci kez** UTF-8'e çevirir, başına `Can't load ` koyar ve `error` başlığı / `0x10` bayraklarıyla `0x180038bc0` yolunu çağırır. Dönüş değeri kullanılmaz ve sonraki DLL denenir. Boş yol dönüşüm yapmadan yine yüklemeye verilir. Metin içinde NUL varsa günlük tam uzunluğu işlerken Win32 yükleyici `c_str()` sonlandırıcısında kesilir.

UTF dönüşüm yardımcısında codepage65001 ve54936 için `0x80`, diğer codepage'ler için `0x400` kullanılır. İkinci grupta default-character kullanımı `{count=0,error=0x459}` üretir. Başarısız çağrıda GetLastError alınır; yalnız `0x3ec` hatasında flags0 ile tekrar denenir. Boş olmayan yolun uzunluğu INT_MAX üstündeyse `0x18000cd70(22)`; dönüşüm hatasında `0x18000cdb0(error)` yolu çağrılır. Bu iki CRT exception ABI'si zorunlu dış sözleşmedir. İkinci dönüşümün boyutu ilk sorgu sonucundan farklı olsa da özgün string uzunluğu ilk sonuçtur; kaynak bunu korur.

Error mode normal dönüşte eski haline getirilir. Kaynak, gözlenen gövdede olmayan exception durumunda otomatik geri yükleme davranışı eklemez. MSVC EH/SEH tablosu ve tüm heap hata ayrıntıları için tam ABI eşdeğerliği iddia edilmez.

Ana faz `0x1800a3ea8`, worker fazı `0x1800a3ec0` listesini alır. Yalnız worker fazı önce `0x180036880(2)` ile bekler. Başarılı handle'lar `0x1800a3ed8` listesinin sonuna eklenir; tekrarlar korunur. Yalnız worker fazı normal sonuna ulaştığında `0x180036820(&byte_1800a3c3e)` çağrılır. Source `Effects.finish_flag_036820` bağlayıcısı bu adresin gerçek shared state'e bağlanmasını gerektirir.

Unload, başlangıçtaki veri adresini ve uzunluğunu saklar; ileri sırada her sıfır olmayan handle için FreeLibrary çağırır ve dönüş sonuçlarını kullanmaz. Listeyi veya girdilerini temizlemez. Bu yüzden ikinci unload aynı handle'ları yeniden bırakmayı dener; kaynak bunu sessizce düzeltmez. Yeni thread kilidi eklenmez. Kaynak nesnelerindeki eşzamanlı erişimi ve özgün mutex/CRT yerleşimini bağlayan entegrasyon henüz yapılmadı. Serbest bırakma sırasında özgün depolama boşa çıkarılırsa erişimin geçerliliği garanti edilmez; yeni bir kopya veya kilit eklenmedi.

## Derleme ve kontrol

`./build.ps1`, mevcut LLVM-MinGW kökünü kullanır; gerekirse `-ToolchainRoot` verilir. İnternet erişimi veya kurulum yapmaz. Clang23.1.3, C++20, -O2, -Wall, -Wextra, -Werror ile çekirdek ve gerçek Win32 adapter iki ayrı nesne dosyasına derlendi.

**157 kontrol geçti.** Yalnız mock API kullanan `libraries_tests.exe` çalıştırıldı. Native adapter test executable'ına linklenmedi. Testler yükleme sırası, başarısızlıktan sonra devam, çift dönüşüm, dönüşüm hata aşamaları, hata modu, flags/fallback, main/worker ayrımı, NUL/boş yol, tekrarlar ve unload sırasını denetler. Depolamayı canlı tutan sentetik yeniden giriş testi, unload sırasında global vector değişse de başlangıç pointer'ının kullanıldığını denetler. Gerçek DLL yükleme/bırakma, UI veya özgün kod çalıştırma yapılmadı. Bu sayı tüm özgün programın test kapsamını ya da diferansiyel davranış eşdeğerliğini göstermez.

`validation.json` derleyici kimliğini, seçenekleri ve kaynak hash'lerini kaydeder. Derlenebilir kaynak mantığı tamamlanmış olsa da bu modülün MSVC STL/exception, günlük locale/stream, MessageBox wrapper ve global ABI bağları açık iştir. Tüm DLL'nin yeniden derlenmiş hali olarak sunulmaz.

## Bağımsız kaynak denetimi

`audit.json`, altı özgün işlevin saklanan assembly ile satır aralıklarını, Evidence kimliklerini ve manuel karşılaştırma sonuçlarını kaydeder. Denetimde unload başlangıç pointer'ının da saklanması düzeltildi; önceki kaynak yalnız uzunluğu saklıyordu. Diğer beş işlevde incelenen kararlar ve API sırası için maddi fark bulunmadı. `native_system.cpp` Win32 argümanlarını doğrudan geçirir; default-character BOOL sonucunu C++ bool'a dönüştürür. Adapter yalnız derlenmiştir. Bu denetim otomatik assembly eşdeğerliği, tüm EH tabloları, allocator büyüme politikası veya çalışma zamanı uyumluluğu kanıtı değildir.

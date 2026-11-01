# NLS API çözümleme ve locale tablosu bağlantısı

Bu köprü `0671d4` ve `0672c0` gövdelerini gerçek `0669b0` çözümleme/önbellek kaynağına ve `07310c` / `0736d4` statik fallback kaynaklarına bağlar. Önceki NLS/record/case/locale/PTD/heap/argv zinciri bu kaynakları birlikte kullanır. Tam DLL veya özgün kaynak/ABI eşitliği iddiası değildir.

| Adres | Gerçek kaynak bağlantısı |
|---|---|
| `0x1800671d4` | Procedure slot `0x11`, `LCMapStringEx`, modül adayları `{3,16}`; export yoksa locale→LCID ve `LCMapStringW` |
| `0x1800672c0` | Procedure slot `0x13`, `LocaleNameToLCID`, aynı adaylar; export yoksa statik isim araması |
| `0x18007310c` | Özgün sıradaki 228 ad/indeks ve 228 LCID kaydı üzerinde ikili arama |
| `0x1800736d4` | İkili aramada85 UTF16 birim karşılaştırma sınırı, yalnız ASCII büyük harf katlama, tam işaretli fark |
| `0x1800669b0` | Önceki gerçek encoded-pointer/module cache ve yordam çözümleme gövdesi |

Çözümleme, önceki runtime `State` nesnesini paylaşır; AreFileApisANSI gibi diğer yordamlar aynı modül/procedure depolarını kullanır. Aday3 localization API-set, aday16 kernel32'dir. Yüklü ilk modülde export bulunamaması resolver'ın mevcut davranışı gereği aramayı bitirir; bu köprü aday listesini yeniden yorumlamaz. Pozitif ve encoded-missing sonuçlar aynı cookie ile cache edilir. Constructor'da preinitialized encoded-null hücreler yeni kaynak bileşiminin sözleşmesidir; özgün image/cookie başlangıcı tamamlanmış sayılmaz.

Ex yolu dokuz argümanı aynen taşır; sürüm/reserved/sort-handle alanları W fallback yolunda kullanılmaz. Ex mevcut olup0 döndürürse W'ye geçilmez. W yolu, locale dönüşümü0 döndürse bile çağrılır. LocaleNameToLCID native yolunda flags korunur; statik fallback flags kullanmaz. Tüm32 bit sonuçlar korunur; pseudocode'daki void işaretleri assembly EAX sonucunu değiştirmez.

228'er16-byte iki tablonun ham aralıkları dosyada `0x8d210` ve `0x8b7c0`, her biri3648 byte'tır. Hashler ve bütün UTF16 isim pointer/byte kayıtları komşu locale-names modülündedir. Özgün tek sıra tersliği `kok-in` ardından `ko-kr` şeklindedir. Aynı ikili arama `kok` ve `kok-in` için0; diğer226 ad için kayıt LCID'sini üretir. `ja-JP→0411`, `ko-KR→0412`, boş ad→007f ve NULL→0 korunur. Bu statik çıkarımdır, özgün DLL üzerinde runtime gözlem değildir. Sabit tabloların runtime mutasyonu kapsam dışıdır.

`NativeCalls`, Ex/W/LocaleNameToLCID native uçlarını zorunlu sağlar. Önceki `NativeEffects` backend'i CP, dönüşüm, GetStringTypeW, scratch ve diğer uçları sağlar; backend'in eski `lc_map_string_0671d4` slotu hiç çağrılmaz. Fixture bu eski slotta exception atarak gerçek yeni yolun kullanıldığını denetler. Yeni adapter çıktı token'larını tracked `ByteOutput` span'lerine taşır ve iç içe çağrı sonrasında önceki span'i geri yükler. Bu token'lar native pointer layout'u değildir. Bağımlılıklar aynı canlı/senkron runtime-global/PTD/heap/bellek alanında tutulmalıdır.

```powershell
& '.\outputs\anadius-crt-nls-dispatch-bridge-source\build.ps1'
```

Strict bağımsız derleme: **27.943 kontrol PASS**. Ex mevcut, Ex yok/native LCID mevcut, iki export da yok/statik fallback, pozitif/eksik cache tekrarları, LCID0 ile W çağrısı, tüm Ex argümanları ve signed dönüş, flags/fullDWORD LCID, özgün iki başarısız isim yolu sınandı. Önceki NLS/record/locale/PTD/heap/argv testleri korunur. Bir Ex callback'i içinde gerçek GetStringType ve map zincirine tekrar giriş yapılır; iç/dış çıktılar, iki scratch frame, input kaydı ve PTD bit2 sahipliği doğrulanır. Bu tek iş parçacıklı sentetik reentry testidir; native concurrency veya özgün SEH/ABI eşitliği değildir.

Native Ex/W/LocaleNameToLCID/GetStringTypeW ve CP/dönüşüm/Heap/FLS/lock/atomik uçları kontrollü fixture'lardır; gerçek OS locale sonuçları, DLL/oyun veya hook çalıştırılmaz. Test backend'i ASCII dönüşümü üretir. Orijinaller değiştirilmez. Native API bağları, image global yaşam süresi, loader, security cookie/CFG, SEH ve tam DLL eşdeğerliği açık kalır. `review-dispatch.json` kaynak/test incelemesini, `validation.json` bağımsız derlemeyi, `common-validation.json` ortak derlemeyi belgeler.

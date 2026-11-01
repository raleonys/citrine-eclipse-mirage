# Yeniden yazılan LSX şifreleme çekirdeği

Bu klasör `anadius64.dll` içindeki AES, challenge ve yanıt anahtarı üretimini derlenebilir C++20 olarak yeniden yazar. Mesaj katmanının `Cipher`, `CryptoFactory` ve `Random` arayüzlerini somut sınıflarla karşılar. Tam DLL veya özgün C++ kaynak kodu değildir; özgün DLL çalıştırılarak davranış eşitliği doğrulanmadı.

Kaynak kimliği: `C:\Users\Skysect\Downloads\anadius64.dll`, SHA-256 `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`. Aşağıdaki VA adresleri bu girdinin `0x180000000` image base değerine göredir. Özgün dosyaya yazılmadı ve dosya yüklenmedi.

## Kaynaktaki davranış

- `Cipher` 16 bayt bloklu, 128 bit anahtarlı, 10 turlu AES uygular. `0x180042780` anahtarı genişletip modu `2` yapar: ECB. `0x180042760` anahtarı ve IV'yi kurup modu `1` yapar: CBC. ECB anahtar değişimi mevcut IV'yi sıfırlamaz. Kurucu/reset modu, 176 bayt round-key alanını ve 16 bayt IV'yi sıfırlar.
- Dize şifrelemesi PKCS#7 doldurması ekler; blok sınırındaki ve boş girdilere de tam 16 bayt doldurma eklenir. Offset öncesindeki önek şifrelenmez. Çözme, blok uzunluğu uygun değilse değiştirmeden `false` döner. Doldurma hatası ise bloklar çözüldükten sonra denetlenir: çözülmüş baytlar ve CBC IV değişimi geri alınmaz. Doldurma denetimi bütün dizenin sonuna bakar; seçilen şifreli aralık boş olduğunda bile bu davranış korunur.
- Sabit mesaj anahtarı `0x180084c98` adresindeki `00 01 ... 0f` baytlarıdır. `set_fixed_key_084c98` adındaki adres bir **veri adresidir**, ayrı bir fonksiyon girişi değildir. S-box, ters S-box ve Rcon tabloları da girdiden bayt bayt doğrulandı; aralıklar ve hash değerleri `constant-evidence.json` ile `evidence.json` içindedir.
- Yerel üretecin 32 bit durumu `state = state * 214013 + 2531011` işlemiyle modulo `2^32` güncellenir. Dönen değer `(state >> 16) & 32767` olur. Bu, süreç genelindeki CRT `rand()` durumu olarak modellenmez.
- `0x180034cf0` zamanı bir kez alır, saniyenin düşük 32 bitini seed yapar ve 16 üreteç sonucunun düşük baytını toplar. Özgün fonksiyon daha sonra `0x18003d190` ile 32 küçük harfli hex karakter üretir. Mesaj arayüzündeki `challenge_bytes_034cf0` yalnız bu **16 ham baytı** döndürür; `messages.cpp` hex dönüşümünü bir kez uygular. `challenge_hex_034cf0` özgün yardımcı fonksiyonun tam hex sonucunu verir. Bu bölünme arayüz tasarımıdır, özgün fonksiyon imzası değildir.
- `0x180034c30` yanıtın ilk iki baytını ayrı ayrı işaretli 8 bit olarak genişletir, ilkini 8 bit sola kaydırıp ikincisiyle OR yapar. Seed `7` ile üretilen ilk sonuç eklenip üreteç yeniden seed edilir; sonraki 16 sonucun düşük baytları anahtarı verir. Kalan yanıt baytları kullanılmaz. Tek karakterli özgün `std::string` için ikinci bayt sonlandırıcı NUL olduğundan kaynak bunu açıkça korur.
- `0x18005eeb8`, `GetSystemTimeAsFileTime` sonucundan `0x19db1ded53e8000` çıkarır, işaretli farkı `10000000` ile sıfıra doğru böler ve saniye `0x793582aff` değerinden büyükse `-1` döndürür. Kaynakta saf dönüştürücü ile `WindowsTimeSource` ayrıdır. Testler sahte saat kullanır; Windows saat adaptörü derlenmiştir.

## Adres ve kanıt haritası

| VA | Yeniden yazılan rol |
|---|---|
| `0x1800423d0` | Kurucu ve yerinde reset |
| `0x180042480` / `0x180042610` | Dize çözme / şifreleme, offset ve doldurma |
| `0x180042760` / `0x180042780` | CBC / ECB anahtar kurulumu |
| `0x1800454e0` / `0x180045580` | CBC blok dolaşımı ve IV güncellemesi |
| `0x180045670` / `0x1800458d0` | AES ileri / ters blok dönüşümü |
| `0x180045a50` | Ters MixColumns; kaynakta ortak sonlu alan yardımcısına katıldı |
| `0x180045ce0` | Anahtarın 44 kelimeye genişletilmesi |
| `0x180045050` / `0x180045060` / `0x180045070` | Üreteç kurma / seed / sonraki değer |
| `0x180034c30` / `0x180034cf0` | Yanıt anahtarı / challenge |
| `0x18005eeb8` | FILETIME → Unix saniye dönüşümü |

`evidence.json`, bu girişlerin ve yönlendirme yardımcılarının toplam 22 adresini hem assembly hem pseudocode Evidence ID'leriyle bağlar. Bunlardan `0x18003d190` destek bağımlılığıdır ve uygulaması mesaj modülündedir; tabloda 22 bağımsız C++ fonksiyonun tamamlandığı iddia edilmez. Ayrıca 17 kesin assembly satırı ve dört sabit veri aralığı vardır. Örnekler:

- `0x180034c55` ve `0x180034c5b`: iki ayrı `MOVSX`; işaretli bayt davranışının kanıtı. Assembly Evidence `ev_a2d44de76b1f36d43c69a61a37f8be0041e16d5cc858db07d098194391164ab0`.
- `0x180045070`–`0x180045080`: çarpma, toplama, durum yazımı, kaydırma ve maske. Assembly Evidence `ev_b754e55552719088c44746338daa208d0549e840cec0375945c9241d689d915f`.
- `0x180042525`–`0x180042567`: son baytın doldurma olarak okunması ve başarısızlık dönüşü; çözme işlemleri bu denetimden öncedir. Assembly Evidence `ev_d0fccb2775fdb815d1a580c813946b1cdefdb26fb8590bbb2b2a03d7b562e8e4`.
- `0x180034dc1`: `0x18003d190` çağrısı; challenge'ın ham baytlarla bitmediğinin kanıtı. Assembly Evidence `ev_947f1c2ad1513d2c702331b9501becbbeebeda26329fb754161419aae3efb063`.

Tam kanıtlar üst klasördeki `anadius-rea-evidence.json`, `anadius-all-functions.asm.txt` ve `anadius-all-functions.pseudo.c` dosyalarında korunur. `evidence.json` bu üç dosyanın SHA-256 değerlerini de içerir.

## Doğrulama ve kapsam sınırı

Yerel `build.ps1` Clang 23.1.3 ile `-std=c++20 -O2 -Wall -Wextra -Werror` kullanır; nesne dosyasını ve yalnız yeni kaynaklardan oluşan sentetik test programını üretir. Testler AES bilinen cevap vektörlerini, ECB/CBC durumunu, bütün 1–16 doldurma değerlerini ve hatalı doldurma konumlarını, offsetleri, hata sonrası mutasyonu, 65.536 ilk-iki-bayt birleşimini, tek baytlı yanıtları ve zaman sınırlarını kapsar. Son çalıştırmanın kesin sonucu ve kaynak hash değerleri `validation.json` içindedir. Test sayısı özgün DLL ile karşılaştırılan davranış sayısı değildir.

`OutOfEvidenceInput` özgün hata ABI'si değildir. Boş şifreli dize, dize dışındaki offset ve baştaki belirsiz bellek baytlarına dayanan boş yanıt gibi girdileri kaynakta açıkça sınırlar. Özellikle boş yanıt için özgün kod ikinci baytı mantıksal dize dışından okur; bu değerin ne olduğu statik kanıtla sabitlenemez. Boş olmayan dizelerde offset sona eşitse özgün doldurma davranışı korunur; doldurmanın öneğe uzanması da ayrıca yasaklanmaz.

Sınıfın özgün bellek düzeni, MSVC `std::string` kapasitesi/SSO temsili, allocation hata davranışı, MSVC exception/unwind ABI'si, ham pointer aliasing ve bozulmuş mod değerleri bu kaynakla eşdeğer ilan edilmez. `0x18005eeb8` içindeki isteğe bağlı çıktı pointer'ı C++ saat arayüzünün parçası değildir. Gerçek XML okuyucu, ağ ve DLL bağlantısı bu klasörün kapsamında değildir. Hedef DLL/EXE ve yeniden üretilen DLL çalıştırılmadı; runtime eşitliği açık kalır.

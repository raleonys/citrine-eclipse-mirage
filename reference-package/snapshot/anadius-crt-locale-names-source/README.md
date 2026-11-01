# Locale adı → LCID statik geri dönüşü

`anadius64.dll` içindeki `0x18007310c` ve `0x1800736d4` gövdelerinin okunabilir C++ karşılığıdır. `locale_names.cpp` özgün kaynak değildir; Ghidra çıktısı, assembly ve doğrulanmış PE baytlarından yeniden kurulmuştur. Orijinal DLL çalıştırılmadı ve değiştirilmedi.

`locale_name_to_lcid_07310c` NULL girdide sıfır döndürür. Diğer girdilerde 228 satır üzerinde özgün ikili aramayı, `0x55` UTF16 birim karşılaştırma sınırını ve işaretli indeks kontrolünü korur. Eşleşmede indeksin gösterdiği DWORD LCID döner. `ascii_wcsnicmp_0736d4` yalnızca ASCII `A`–`Z` birimlerini küçültür ve katlanmış unsigned UTF16 değerlerin tam işaretli farkını döndürür. Unicode büyük/küçük harf dönüşümü yapmaz.

## Veri kanıtı

`extract-tables.cjs` girdinin SHA-256 değerini `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c` olarak doğrular. PE32+ image base ve section raw aralıklarını okuyarak şu iki aralığı eşler:

| Tablo | VA | Dosya ofseti | Boyut | SHA-256 |
|---|---|---|---|---|
| Ad pointer / indeks | `0x18008ea10` | `0x8d210` | 3648 | `477758760a6b04a3b7f198874408f4fc9156795260b28f26da940c5686a4b9f6` |
| LCID kayıtları | `0x18008cfc0` | `0x8b7c0` | 3648 | `12c490beaf37c4da406d63ec5c041966e63975a7221100893416a85475497245` |

Her tabloda 228 adet 16 baytlık kayıt bulunur. `extracted-tables.json` tüm kayıtların ham hex değerini, isim pointer/indeksini, adres→dosya ofseti eşlemesini ve her NULL sonlandırılmış UTF16 ismin ham baytlarını/hash değerini içerir. Üretim kodu yalnızca yordamın okuduğu ad/indeks ve LCID DWORD alanlarını kullanır. Diğer alanlar kanıtta saklanır. Çıkarıcı salt okunur girdiden aynı JSON ve C++ dosyasını yeniden üretir; saklanan pseudo/assembly dosyalarının SHA-256 değerlerini de doğrular.

## Korunan özgün sıra kusuru

Tablo sırası ASCII karşılaştırmaya göre tek bir yerde ters dönmektedir: sıra 129 `kok-in`, sıra 130 `ko-kr`. Özgün ikili arama bu sırayla çalıştığında 128 numaralı `kok` ve 129 numaralı `kok-in` satırlarına erişemez ve sıfır döndürür. Diğer 226 ad kayıtlarındaki LCID değerine ulaşır. Tablo yeniden sıralanmadı. `ko-KR → 0x412`, `ja-JP → 0x411`, boş isim `→ 0x7f`, NULL pointer `→ 0` örnekleri ayrı doğrulanır. Bu sonuçlar statik akıştan hesaplanmıştır; orijinal üzerinde dinamik gözlem değildir.

## Sözleşme ve kalan sınırlar

`Effects::read_u16` girdinin okunabilir UTF16 belleğini sağlar. Sağ operand C++ görünümü, NULL sonlandırılmış sabit string literal tarafından desteklenir; bu mantıksal arayüz özgün x64 ABI eşitliği iddiası taşımaz. `count == 0` hiçbir operandı okumaz. NULL olmayan ve okunamayan girdiler için özgün erişim ihlali davranışını C++ istisnasına dönüştürme politikası tanımlanmadı. Yordam errno/Win32 API kullanmaz.

Gömülü tablolar özgün baytlarla sabittir. Başka kodun çalışma sırasında bu statik tablo alanlarını değiştirdiği durumlar bu sürümün kapsamı dışındadır. Signed indeks ret dalları kaynakta korunur; sabit tablodaki tüm indeksler geçerli ve 0..227 permütasyonudur, bu yüzden bu ret dalları tablolar değiştirilmeden erişilebilir değildir.

`build.ps1`, mevcut LLVM ile C++20 `-O2 -Wall -Wextra -Werror` üretim nesnesini ve statik bağlı sentetik test programını derler. Tüm adlar, karışık harf büyüklükleri, proper prefix girdiler, bilinmeyen ekler, özgün iki başarısız arama, ASCII katlama sınırları ve 65536 UTF16 değeri dokuz sağ operand uç değeriyle denetlenir. Windows locale API'leri çağrılmaz.

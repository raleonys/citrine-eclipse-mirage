# Locale ve LC_TIME referans sayaçları

`locale_ref.cpp`, `anadius64.dll` içindeki dört gövdenin bağımsız C++20 yeniden yazımıdır. Özgün kaynak/ABI eşitliği veya DLL bütünlüğü iddiası yoktur. Aynı dosyanın SHA-256 kimliği `evidence.json` içinde sabittir.

| Giriş adresi | Kaynak işlevi | Korunan davranış |
|---|---|---|
| `0x180070634` | `add_locale_ref_070634` | Locale +0x10; dört isteğe bağlı sayaç; altı kategori; LC_TIME artışı |
| `0x1800708c0` | `release_locale_ref_0708c0` | NULL ise işlem yok; aynı sırada azaltma |
| `0x180070838` | `add_time_ref_070838` | NULL/statik time için INT_MAX; diğerlerinde +0x15c atomik artışı ve yeni değerin tam 32 biti |
| `0x180070898` | `release_time_ref_070898` | NULL/statik time için INT_MAX; diğerlerinde +0x15c atomik azaltması ve yeni değerin tam 32 biti |

İsteğe bağlı sayaçların yükleme sırası `locale+0xe0`, `+0xf0`, `+0xe8`, `+0x100` şeklindedir. Kategori imleci `locale+0x38` ile başlar ve altı kez 0x20 artar: imleç−0x10 (`locale+0x28`) kategori adı statik `0x1800a1948` kimliğine eşit değilse imleçteki isteğe bağlı sayaca erişilir; imleç−0x18 (`locale+0x20`) sıfır değilse imleç−8 (`locale+0x30`) içindeki isteğe bağlı sayaca erişilir. Bu alanlar gerçek assembly ofsetleridir. Ardından `locale+0x120` yeniden okunup time yordamı çağrılır. Statik time kimliği `0x18008ad20` değeridir.

`Identities` bu iki özgün statik adresin yeniden kurulan görüntüdeki karşılığını taşır. `Effects::read_address` canlı ve geçerli adres verisini okumalıdır; `atomic_fetch_add_32` bir adet atomik modulo-2^32 güncelleme yapıp eski değeri dönmelidir. Bu sağlayıcılar soyuttur. Kaynak doğrudan keyfi adres dereference etmez, native API çalıştırmaz. Sayaç sıfıra inse bile bu dört işlev bellek serbest bırakmaz. Ayrı locale/time serbest bırakma gövdeleri bu modülün kapsamı dışındadır.

Kaynak yüklemeleri önden toplamaz: her yükleme önceki atomik etkiden sonra yapılır. Böylece takma adresler ve sonraki işaretçiyi değiştiren sağlayıcılar assembly sırasını korur. Locale ekleme işlevine NULL verilmesi özgün gövdede korumasızdır; test bu erişimi fixture hatası olarak yakalar. Yeni başarı/hata kodu veya geri alma yolu eklenmemiştir. Provider exception/erişim ihlali ve SEH/unwind özgünle eşdeğer kabul edilmez.

`build.ps1` mevcut LLVM-MinGW ile bir üretim nesnesi ve sentetik test EXE'si derler (`C++20 -O2 -Wall -Wextra -Werror`). Testler dört isteğe bağlı sayacın tüm 16 maskesini, her kategoride sentinel/NULL/dolu kombinasyonlarını, tam etki sırasını, ortak sayaç adreslerini, aradaki etkilerin sonraki işaretçileri değiştirmesini, NULL ve statik time durumlarını, INT32 sınırlarını ve unsigned taşmayı denetler. Hedef DLL yüklenmez/çalıştırılmaz; gerçek locale/heap/atomik sağlayıcı kullanılmaz. Sonuç `validation.json` içindedir.

`selected.asm.txt` ve `selected.pseudo.c` önceki REA/Ghidra çıktısından yalnızca bu dört işlevin kanıtını içerir. `collect-evidence.ps1` özgün DLL ve korunmuş tam çıktıları SHA-256 ile doğrulayarak bu sınırlı seçimi yeniden üretir. Bu işlem yeni Ghidra analizi değildir; güncel sunucu/Ghidra bağlantısının doğrulaması üst çalışma tarafından yapılmıştır.

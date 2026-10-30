# Windows kabulü ve DLL boyutu

Kullanıcının sunduğu Compile, Production, Debug, Static CRT ve Native logları PASS ile kapanıyor. Native193/193, CRT6/6 ve Production'ın altı Golden kapısı başarılı. Yüklenen DLL'in SHA-256'sı Production logundaki nihai DLL ile aynı:

`4a0f37c9328ecf31d0815a9e9b85634bc481e77e2f07b636d4ea65bb4783f26b`

Ayrıca yüklenen DLL üzerinde salt okunur final metadata incelemesi PASS: doğru timestamp, geçerli checksum, Rich/debug/COFF symbol/certificate yokluğu, üç doğru C export/ordinal, istenen import sırası, static CryptQueryObject import ve iki doğru read-only avatar doğrulanır. PE resource yalnız beklenen VERSIONINFO'dur.

Oyun kabulü kullanıcının açık PASS beyanı olarak kaydedildi; oyun adı veya her senaryonun ayrı matrisi verilmedi ve burada oyun tekrar çalıştırılmadı. Önceki native/Golden/oyun bekleme kayıtları tarihsel kanıt olarak korunur; güncel durum [kabul kaydındadır](evidence/OWNER_NATIVE_ACCEPTANCE.json). Yeni DLL için önceki üç bekleyen kabul işi bu kanıt sınıflarıyla kapanmıştır. Bu, bütün özgün DLL fonksiyonları/ABI'si veya tüm OS/consumer senaryoları için sınırsız eşdeğerlik iddiası değildir.

## Ölçülen fark

| Bölüm / dosya | Özgün DLL, KiB | Ürün DLL, KiB | Fark, KiB |
|---|---:|---:|---:|
| Dosyanın tamamı | 678 | 880 | +202 |
| .text: makine kodu | 501 | 626,5 | +125,5 |
| .rdata: sabitler, tablolar, C++/exception bilgileri | 135,5 | 185 | +49,5 |
| .data: dosyada bulunan değiştirilebilir veriler | 10 | 26 | +16 |
| .pdata: x64 unwind kayıtları | 25 | 34 | +9 |
| .reloc | 4 | 6 | +2 |
| .rsrc | 1 | 1 | 0 |

KiB=1024byte; tabloda PE bölümlerinin dosyada kapladığı raw size kullanılır. Dosya farkı206848byte, yani202KiB ve%29,7935. Başlık boyutları aynı; üründeki0,5KiB .fptable ve özgündeki0,5KiB _RDATA da toplam farkta birbirini dengeler. İlk beş büyüyen bölüm farkın tamamını açıklar. Bu dosya boyutudur; private working set/RAM ölçümü değildir.

Ürün iki geçerli PNG içerir:4181+3882=8063byte. Özgünde7488+3882=11370byte vardır. Avatar verisi3307byte küçülmüştür. Resource bölümü aynı1KiB; avatar PE resource'a konmamıştır. Üründe debug dizini/COFF symbol tablosu ve bölümlerden sonra overlay bulunmaz. Dolayısıyla bu büyüme avatar, PDB gömülmesi veya dosyaya eklenmiş artı veriden gelmiyor.

## Neden makul, ne kadarını kesin biliyoruz?

Kaynakta açıkça onaylanan ek runtime davranışları bulunuyor: hook11/SAFE-FREEZE, call-gate ve retryable cleanup, atomik dosya/rollback sahipliği, bounded worker/oturum/deadline/event delivery yönetimi ve artık iki ailede çalışan listener. Modern vendor sürümleri, compiler-owned standard CRT/STL, template/inlining tercihleri ve C++ exception/RAII kodu da özgün binary ile farklı bir codegen üretir. Bunlar ek kod/veri üretebilecek somut kaynak farklarıdır;202KiB'ın her kütüphaneye kesin dağıtımı değildir.

İki DLL'in de doğrudan dinamik CRT import'u yoktur; yalnız “biz statik CRT kullanıyoruz” açıklaması farkın tamamını kanıtlamaz. Mevcut Golden link profilinde /LTCG:OFF açıkça seçilidir; özgünün gerçek optimizasyon/LTO ayarları DLL başlığından çıkarılamaz. Üründe2869, özgünde2130x64 exception-directory girdisi vardır; bu daha fazla unwind bölgesiyle tutarlıdır ama739bağımsız ek fonksiyon kanıtı değildir.

Production ve Native loglarında gerçek target/linker-map sınırı PASS;74referenceTU'nun ürüne karıştığına dair bir bulgu yok. Herhangi bir unreachable/dead code olmadığının ispatı olarak yorumlanmaz. Tam fonksiyon/kütüphane boyut hesabı için kabul edilen aynı native build'in linker MAP, OBJ/LIB veya PDB katkı bilgileri gerekir; bunlar yüklenmedi. Mevcut DLL'lerle tam byte-attribution iddia edilmez.

Boyut farkı mevcut ek davranışlar ve farklı build bileşimi için makuldür; tek başına çalışma hatası veya gereksiz source import'u göstermez. Bu incelemede runtime kodu, vendor byte'ları, linker/optimizasyon seçenekleri ve teslim edilmiş ZIP'ler değiştirilmedi. Yalnız yeni kanıt ve güncel kabul durumu kaydedildi.

Native logda12compiler uyarısı vardır; verilen konumlar test/fixture kaynaklarına veya onların template instantiation zincirlerine aittir. Native test sonucu başarısız değildir. Compiler uyarısız kabul iddiası yapılmaz; bu tur boyut sorusu nedeniyle test kodu da değiştirilmedi.

[Byte ve bölüm dökümü](evidence/OWNER_DLL_SIZE_COMPARISON.json) · [Final DLL incelemesi](evidence/OWNER_FINAL_DLL_INSPECTION.json) · [Ham log ve kabul özeti](evidence/OWNER_NATIVE_ACCEPTANCE.json)

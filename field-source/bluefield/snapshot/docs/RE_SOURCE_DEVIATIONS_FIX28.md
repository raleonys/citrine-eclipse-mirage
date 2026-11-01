# FIX28 — Config2 için canlı kaynak referansı ve doğru failure olayları

NON_RELEASE tam kaynak/build/QA devam paketi, exact FIX27 ebeveyninden türetildi. Karşılaştırma kullanıcının RE C++ paketidir; özgün DLL binary eşdeğerliği değildir.

## Giderilen QA borcu

Eski Config2 differential testi8766 dondurulmuş FIX09 expected çıktısına bakıyordu. Bu değerler regresyon kanıtıdır; kendi başlarına bağımsız kaynak doğrulaması değildir. FIX28 paket anadius-parser-source/parser.hpp/cpp dosyalarını ayrı test namespaceinde gerçekten derler. Include relocation ve namespace dışında bütün body bytesı aynıdır; ters dönüşüm orijinal dosyayla birebir eşleşir. Test referansı production DLL graphına girmez. Referansın parser State/Node/algoritması adaydan ayrıdır; dormant configuration conversion API için mevcut config tipi kullanılır ve bunun sınırı binding kaydında açıklanır.

Yeni test eski corpusun sadece input sütununu alır, expected sütununu okumaz. Canlı source parse sonucu PEGTL sonucu ile77205 örnekte karşılaştırılır. Logical tree, assignment sırası, incomplete/open frames, accumulator/key/value, BOM, hata kuralı ve üç koordinat dahildir. Kısa byte stringler, iki grammar contexti ve art arda reused-State örnekleri eski corpusun ötesine geçer. Bu sonucun anlamı source model ile aktif parserın incelenen sözleşmesidir; orijinal binary exception ABI veya bütün sonsuz input uzayı değildir. Parser üretim kodu değiştirilmedi.

Önceki52 C++ test/header bytesı korunur. Tüm current C++ test/support dosyaları conservative rol tablosuna bağlanır. Packet ancestryli uyarlanmış fixturelar literal independent oracle sayılmaz; app expectations, frozen self regression, source fragments, native fixtures ve owned integration açıkça ayrılır. Rol metadataı kendi başına expectation onayı üretmez.

## Somut log hatası ve korunan privacy tercihi

Windows bootstrapın iki exception catchi hata olduğunda startup olayını yazıyordu; cleanup catchi phase yazıyordu. Artık sabit startup_failed ve shutdown_failed etiketleri var. Enum değerleri sona eklendi; önceki ordinal değerleri değişmedi. Exception/stop/cleanup kontrol akışı, timeout/looplar ve mevcut exit davranışı aynıdır.

Debug loguna what/raw XML/token/config/path dökülmez. Mevcut network_parse/network_crypto/network_failure sabit kategorileri privacy tercihi olarak korunur ve tam orijinal diagnostic-text eşdeğerliği diye sunulmaz. Production disk log kapalıdır. Gerçek host logger testinde fixed event formatı ve Production dosyasızlık doğrulanır. Windows catchlerin actual native yürütmesi NOT_RUNdır; source binding ve negatif kontroller onun yerine native PASS yazmaz.

## Durum ve sınır

Q02 bounded source-QA/diagnostic incelemesi kapandı: frozen-only referans eksikliği ve yanlış failure etiketleri giderildi, kalan fixed-category privacy tercihi açıkça review edilip korundu. Açık9→8: DEP05, D02, D06, D07, D13, D14, D15, D21. Bu kapanış bütün testlerin bağımsız, bütün runtime farklarının çözülmüş veya Windows/game kabulünün verilmiş olduğu iddiası değildir.

119 diğer runtime dosyası,1060 vendor, parser/CFG/default/avatar/persistence algoritmaları ve bütün önceki Golden/CMD/PS1 araçları değişmez. Final QA sonuçları ayrı rapordadır; Windows/MSVC/provider/CRT/IOCP/game, actual DLL Golden ve LSan NOT_RUN.

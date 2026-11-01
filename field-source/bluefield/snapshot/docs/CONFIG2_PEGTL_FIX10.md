> Tarihsel aşama kaydı. Güncel FIX13 davranışı ve geri alınan sınırlar için ORIGINAL_DEVIATION_AUDIT_FIX13.md geçerlidir.

# FIX10 — Config2/VDF üzerinde gerçek PEGTL

## Değişiklik

FIX09'un elle yazılmış byte matcher'ı yerine pinned PEGTL combinator grameri ve
ayrı owned action/control adapter'ları çalışır. Application::load_document aynı
parser::parse girişini çağırır; eski parser veya config ile backend seçimi yoktur.
BROWNFIELD application parser C++'ı kopyalanmamıştır.

Kullanılan upstream commit `3b075432e94f6f906425350b43d03cd9cebfa4f1`;
HF19 pristine include ağacı, lisansı ve commit kaydı byte-identical'dır. Header
version 4.0.0 der; bu main/prerelease snapshot'tır, Anadius'un özgün PEGTL sürümü
olduğu veya güncel en yeni sürüm olduğu iddia edilmez. C++20 gereksinimi mevcut
proje standardıyla aynıdır. Vendor yerel yama içermez; offline pakete dahildir.

## Korunan davranış

- Root yalnız quoted object'ler, separator ve EOF kabul eder.
- Scalar key/value arasında en az bir separator gerekir; tamamlanmış üyeler
  birbirine bitişik olabilir. ASCII HT/LF/VT/FF/CR/SP korunur.
- // yorumu LF/CRLF veya EOF; /* */ ilk terminatörle biter, nested değildir.
- Yalnız escaped quote/backslash decode edilir. Diğer escape'ler, NUL ve geçersiz
  UTF-8 byte'ları literal kalır; raw CR/LF quoted string içinde hatadır.
- Bir UTF-8 BOM silinir. UTF-16 veya ikinci BOM için yeni dönüşüm eklenmemiştir.
- Son tamamlanmış aynı isimli node tüm önceki subtree'yi değiştirir, merge etmez.
- PEGTL cursor rewind action durumunu geri almaz. Başarısız object-name denemeleri
  açık frame bırakabilir; completed_assignments ve partial state korunur.
- LF-only position tracking, platforma bağlı default EOL seçimini önler. Hata
  koordinatları tek BOM silindikten sonraki byte/line/column değerleridir.
- Üç owned ParseError kategorisi ve mevcut çağıran/diagnostic sözleşmesi korunur.

Optional base/override, constructor defaults, profil adı SURGE, sayısal kimlikler,
SDK ve diğer ayarların uygulama sırası değişmez. Mevcut Application 1 MiB config
ve 128 nesting ön kontrolü değişmez. Hash/list iteration adapter'ı da değişmez;
PEGTL AST metin sırasını özgün Windows container sırası saymaz.

## Kanıt ve kapsam

Özgün `0734efcc...` paketin anadius-parser-source/README.md, evidence.json ve
parser.cpp dosyaları gramer/action dayanağıdır. Windows PEGTL rule RTTI ve Mac
vdf::parser::Grammar sembolleri backend tanımını destekler; Mac sürüm eşitliği
ve Windows orijinal exception ABI'si bu çalışmayla kanıtlanmaz.

Mevcut 115 parser assertion'ı korunur. Ek sabit corpus, **8766** girdide FIX09'dan
alınmış tam result/state snapshot'larını yeni PEGTL çıktısıyla karşılaştırır:
4322 prefix, her byte değeriyle quote/separator/comment denemeleri, BOM varyantları,
deterministik bozuk girdiler, mutasyonlar, duplicate/nested/wide ağaçlar ve
64/127/128/129 derinlikleri. Aynı girdiler corpus içinde saklıdır; yeni parser'dan
expected çıktı üretilmez. Corpus yalnız test verisidir; eski matcher üretim veya
test paketi içine fallback olarak taşınmamıştır. metadata/CONFIG2_CORPUS_FIX10.json
baseline ZIP/parser, snapshot formatı ve corpus hash'lerini bağlar.

Profile/config 53-check integration ve gerçek Asio loopback testleri de aynı
aktif backend ile yeniden çalıştırılır. CMake/portable derleme explicit pinned
include kullanır. Source policy PEGTL bağlantısı, gerçek parse çağrısı ve corpus
kimliğinin kaldırılmasını negatif mutasyonlarla reddeder.

Bu kabul edilmiş byte davranışının backend geçişidir; orijinal native exception
text/RTTI/unwind, allocator davranışı, özgün PEGTL sürümü veya tüm VDF lehçeleri
ile eşdeğerlik sonucu değildir. Yeni config anahtarı, SDK seçimi, comment/nesting
özelliği, encoding dönüşümü veya filesystem backend'i eklenmemiştir.
Windows MSVC/Native/CRT ve gerçek DLL Golden kabulü ayrıca gereklidir.

# B32 — Session control gerçek temizleme ve silme zinciri

Bu paket DLL kritik yolundaki session yaşam döngüsünü ilerletir. **Dört typed normal
gövde** eklendi: gerekli çıkış-kuyruğu temizliği026880, resource dispose026570,
nullable virtual delete017420 ve flags/sizedfree026290. Yeni somut
`RecoveredSessionControl`, B30 gerçek023000 factory'sinde kullanılabilir.
Production SessionAllocator/free domain ve bütün server kaynak grafiği henüz tamam değildir.

## Kanıtlı kaynak sırası

026570 CONTROL tabanından çalışır: control+d8(owner+c8) çıkış deque'ini026880 ile
temizler; canlı proxy'yi alıp NULL yazar ve16 byte sizedfree çağırır; callback'ten
sonra input'u yeniden okuyup heapini bırakır; yalnız size0/cap15/ilkbyte0 resetini
yapar; gerçek socket024db0'ı çalıştırır; ardından CANLI selfweak'i düşürür ve son
weak'te slot08'i çağırır. Native kod selfweak object/control alanlarını sıfırlamaz.
Bu owned selfweak düşüşü, dış strong-release akışının implicitweak düşüşünden öncedir.

026880 mantıksal elemanları sondan temizler; seçili string adresini callback boyunca
cache eder, free sonrasında canlı count'u okuyup azaltır. Başlangıç count0 ise
offset korunur. Blok döngüsünün ilk map-size sınırı cache edilir ve signed JLE/JG
ile yorumlanır; her tur map canlı okunur. Blok free isteği32, map free isteği
CANLI size<<3'tür (uint64 wrap). Nonnull fakat zero-length map de free(bytes0)
yolunu kullanır. Büyük string/map tahsislerinde data-8 header, unsigned padding<=31
ve +39 sized-byte düzeni korunur. Kalan inline byte'lar topluca sıfırlanmaz.

017420 NULL'da döner; diğer durumlarda canlı slot10'u flags1 ile çağırır.
026290084750 table kimliğini flags testinden önce geri yazar; bit0 varsa native
0x1c8 isteğiyle free çağırır. Free sonrasında nesne okunmaz; dönen sayısal kimlik
ölmüş depolamayı gösterebilir. Table kimliği callable native vptr değildir.

## Doğrulama ve sınırlar

54 core kontrol, sıra/prefix, callback sonrası canlı okumalar, cache edilen
string/blok döngüsü, threshold/uint64 arithmetic, flags, null ve throwing lower
prefixlerini sınar. Huge/malformed metadata senaryoları bounded arithmetic
projeksiyonlarıdır; native bozuk pointer davranışı iddiası değildir.

43 integration kontrolü B30 gerçek023000 factory'sinin **final üretim kaynak
RecoveredSessionControl** nesnesini kullanır. Gerçek read/027710 EOF,
025480→021770→028d30, GetConfig/XML/cipher ve kontrollü responsejob zinciri
çalışır. Kuyruktaki heap payload, aktif ve kullanılmayan bloklar, map, proxy,
input, socket ve selfweak bırakılır; observerweak varken control yaşar, son weak
017420→026290 üzerinden gerçek typedC++delete yapar. Önceki custom host disposer
bu testte kullanılmaz. Allocation provenance ve response scheduler kontrollü
sağlayıcılardır; actual IOCP/network veya arbitrary detached concurrency kanıtı yoktur.

Native request1c8, sizeof(hostpolymorphicclass) değildir. Borrowed context,
allocator, map/string projeksiyonları ve socket servisi son weak'e kadar yaşamalıdır.
Native MSVC layout/vtable/STL, allocatorABI, EH/SEH rollback, rawjobquiescence,
unload ve loader-safe attach/detach açık kalır. B31 DLL hook'ları başarı stublarıyla
kapatılmaz; toplam DLL tamamlanma yüzdesi bu sayaçlardan türetilmez.

846 önceki common kaynak değişmedi. İki değişmeyen B31 gate CPP/HPP kaynağı ilk
kez common snapshot'a alınır; bunlar B32'nin dört yeni native gövdesi sayılmaz.
Canonical envanter490→494 mapped,494→498 eligible ve127→131 normal claim bekler;
gerçek nativeABI/provider kapanışı0 ve52 tarihsel kapanış korunur.

Retained pseudo/ASM, Evidence ID, satır ve SHA'lar evidence.json/source-audit.json
içindedir; bu aşamada orijinal byte/decompile sorgusu yapılmadı. Orijinal veya rebuilt
DLL/oyun çalıştırılmadı; gerçek CFG/token/achievement içerikleri okunmadı.
Generic CRT kuyruğu duraklatılmıştır. Ortak build'in kesin sonucu aşağıya eklenir.

## B32 ortak doğrulama

**54 çekirdek + 43 entegrasyon = 97 kontrol PASS.** Ortak build 249 nesne (245 C++ / 4 C), 293 normal test grubu ve 19 framework terminal senaryosu içerir. Önceki 846 kaynak aynı; 0 önceki dosya açık hash kaydıyla güncellendi; toplam 852 kaynak. Yeni typed normal gövde 4, yeni native ABI/provider kapanışı0; 494 adres, 498 karşılaştırılabilir derlenmiş etiket ve52tarihsel kapanış. Bileşim ile raw-all defterleri ayrıdır. Gerçek alt sağlayıcılar, foreign ABI, image unload ve PE girişi açık kalır.

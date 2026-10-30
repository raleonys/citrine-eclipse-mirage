# Güncel DLL C arayüzü

Son kullanıcı talimatıyla Production Brownfield’in üç export/ordinal düzenine
döndü: **DllMain@1, SURGE@2, DllShutdown@3**. DllWaitReady@4 ve DllServerPort@5,
kodları ve public bildirimleri kaldırıldı. C++ kaynak API’leri ayrı
preloader.source.dll geliştirme hedefinde kalır.

DllMain loader tarafından CRT entrypoint’i üzerinden çağrılır; araçlar onu
manuel çağırmaz. SURGE RET-only branded import işaretidir. DllShutdown mevcut
0..4 durum kodlarıyla gerçek worker/IO/hook/provider cleanup sonucunu verir.
Yükleyen taraf successful shutdown öncesi LoadLibrary referansını bırakmamalıdır.

Runtime test host’u iki yönetim export’una ihtiyaç duymaz. DLL’in zaten ürettiği
EALsxPort ortam değişkenini ve gerçek LSX Challenge/GetProfile akışını gözleyip
üç load/start/socket/profile/shutdown/unload çevrimi yürütür. Yanlış/eksik
başlangıç ağ yanıtı olmadan PASS sayılmaz. C17 consumer da üç-export import
kitaplığına bağlanır ve gerçek worker shutdown/idempotence denetimi yapar.

İsimle lookup için export adı, ordinal ile lookup için sayısal kimlik esas olur.
Araçlar isim tablosunu alfabetik gösterebilir: DllMain, DllShutdown, SURGE yanında
1,3,2 görünmesi Brownfield düzeniyle uyumludur. .def satırları ordinal sırasıyla
yazılır. Bu sıra DLL başlangıcı veya hook aktivasyonu sırası değildir.

Orijinal Anadius CDX@1/RUNE@2/anadius@3 işaretleri aynı RET gövdesine gider;
istenmeyen bu adlar geri eklenmez. Özgün consumer’ın anadius64!anadius importuyla
otomatik uyumluluk iddia edilmez. Önceki 548-byte ABI iddiası kaynak kanıtı
bulunmadığından geri çekilmiştir. Gerçek oyun açılışını kullanıcı parent pakette
doğrulamıştır; güncel IO/ABI paketiyle Windows tekrarı ayrıca gerekir.

Tam sözleşme metadata/PUBLIC_DLL_ABI.json içindedir.

Kaynak seviyesindeki durum türü `DllShutdownStatus`, sabitleri
`DLL_SHUTDOWN_OK`, `DLL_SHUTDOWN_CONCURRENT`, `DLL_SHUTDOWN_TIMEOUT`,
`DLL_SHUTDOWN_WAIT_FAILED`, `DLL_SHUTDOWN_CLEANUP_FAILED` olarak adlandırılır.
Değerleri sırasıyla0..4 kalır; bunlar ek DLL export'u değildir.

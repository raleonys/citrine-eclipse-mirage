> Güncel bağlantılar ve F01–F04 düzeltmeleri: [CURRENT_RUNTIME.md](CURRENT_RUNTIME.md).
> Aşağıdaki gövde önceki görevin tarihsel kaydıdır; eski export/resource anlatımı güncel sözleşme değildir.

> Bu bağlantı görevinin tarihsel kaydıdır. Windows IO ve vendor seçimi için
> son kullanıcı düzeltmesi [WINDOWS_IO_FIX.md](WINDOWS_IO_FIX.md) içindedir.

# Uygulama bağlantıları — YF-RUNTIME-BIND-20261028

Kullanıcı, karşılaştırmada eksik görünen bağlantıların zaten yapılmış olması
gerektiğini belirtti. Bu görev somut uygulama bağlantılarını tamamlar; yalnız
rapor veya yeni soyut port eklemekle tamamlanmış sayılmaz.

Önceki karşılaştırmada bir hata: `ScalarDependencies` sınırlı bir alternatif
köprüydü. `config_application::read_config2/apply_config2` ve `FilePass` tam
hash-capacity/section-copy yolunu zaten sağlıyordu. Sekiz çocuk veya scalar-only
sınırı bütün config uygulamasına ait değildi. Yeni owner doğrudan tam yolu kullanır.

Uygulama değişiklik alanları: `src/net/asio_service.cpp`,
`src/platform/file_io_windows.cpp`, `src/runtime/application*.cpp`, ilgili
başlıklar, DLL entry/management API, CMake ve gerçek uygulama entegrasyon testleri.
Kaynak işlevlerin temeli mevcut RE modülleri ve `FUN_180036d20` constructor'ının
sağlanan pseudo/asm kanıtıdır. Özel CRT veya karşılaştırma uygulama kodu alınmaz.

Ürün uyarlamaları açıkça ayrı tutulur: SURGE görünen ad, LocalAppData/SURGE,
SURGE PNG ve Debug-only sabit olay logu; RE executable-relative main→override
sırası `surge.cfg`→`surge_override.cfg` adlarıyla kullanılır. Provider mevcut
RE version_check=2/create_online/vtable sözleşmesiyle yeni isim altında ayrı
adapterdır. Özgün native ABI eşdeğerliği bu bağlantıdan çıkarsanmaz.

Sahiplik: tek Application config/effect/event/provider/hook/service ömrünü tutar;
Asio session callback ve işler session'ı canlı tutar. İşler gerçek standart CRT
thread'leridir; kaynak frame-başına iş modeline sabit worker havuzu eklenmez.
Cipher per-session korunur. Stop, admission'ı kapatır, socket'leri iptal eder,
IO ve handler işlerini bekler; kaynaklar sonra bırakılır. Native hook rollback,
quiescence ve gerçek Windows kabulü ayrıca kaydedilir.

DLL attach yalnız worker için Windows handle'larını kurar. Ağ, config, hook,
provider veya C++ Application kurulumunu loader lock altında çalıştırmaz.
`DllWaitReady`, `DllServerPort`, `DllShutdown` bu yeni
uygulamanın yönetim API'sidir. Sonraki C arayüzü göreviyle SURGE import işareti
ve gerçek DllShutdown adaptörü eklendi; C++ kaynak API'leri ayrı geliştirme
DLL'ine taşındı. Önceki 548-byte ABI iddiası kaynakla doğrulanamadığı için geri
çekildi. Güncel sözleşme: [PUBLIC_DLL_ABI.md](PUBLIC_DLL_ABI.md).

Doğrulama hedefi: gerçek vendor-backed config/Unicode dosya IO, gerçek yerel
soketten XML/LSX profile/content/achievement istekleri, DLL yüklemesiyle otomatik
başlatma, hata prefix rollback ve bekleyen socket/handler işlerinde stop. Wine,
yerel Windows/MSVC ve gerçek Consumer/oyun kanıtı ayrı sınıflardır.

Abort'ta admission kapandıktan sonra özel source wait gate'leri bırakılır;
henüz tamamlanmamış sonraki fazı bekleyen native callback'ler drain'i kilitlemez.
Bu işlem başlangıç başarısı değildir: `start()` hata verir, DLL wait API'si de
başarı döndürmez. Kısmen etkinleşmiş hook kümesi hazır uygulama sayılmaz.

Ek DLL fazları gerçek `load_phase_035850/load_list_035260/unload_035ab0`
işlevlerine bağlıdır. Her başarılı OS load'u source listesine ekleme allocation'ı
başarısız olsa bile owner tarafından izlenir; cleanup lease'leri bir kez bırakır.

Kullanıcının sonraki düzeltmesiyle Brownfield'in `resources/preloader_version.rc`
dosyası birebir alındı. Sabit VERSIONINFO numerikleri, dört string alanı ve
0409/1200 translation profili korunur. Son kullanıcı düzeltmesiyle PNG artık PE resource değildir; eski profil avatar verisinin yerine read-only C++ veri olur. Ayrı fallback PNG, RE kaynağından korunur. [Güncel düzeltme](GOLDEN_AVATAR_FIX.md).
Seçili sözleşme: [RESOURCE_METADATA_CONTRACT.json](../metadata/RESOURCE_METADATA_CONTRACT.json).
`inspect_dll.py` yalnız görseli değil gerçek PE içindeki VERSIONINFO alanlarını
ve iki resource'tan oluşan allowlist'i birlikte denetler. Bu resource profili
diğer PE header/ABI/Golden politikalarını otomatik etkinleştirmez.

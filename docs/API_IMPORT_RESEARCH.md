> Bu araştırma, tam kopyalama yardımcısı eklenmeden önceki kaynak snapshot’ını inceler.
> Güncel uygulama: [RE kopyalama kurtarımı](RE_COPY_RECOVERY.md).
> Aşağıdaki metinde geçen probe/JSON dosyaları, ayrı teslim edilen
> surge-api-import-investigation.zip araştırma paketine aittir.

# SURGE / Anadius: altı API farkının kaynak ve bağlantı incelemesi

29 Ekim 2026. Ürün kodu değiştirilmedi. Bu rapor önceki import değerlendirmesindeki
belirsizlikleri daraltır; yalnız import adına göre verilen öncelikleri doğrulamaz.

## Dört eksik görünen API aktarımda mı unutuldu?

Bu dört import, unutulmuş dört uygulama işlevi anlamına gelmiyor. İkisinde eski
kütüphane uygulaması değişmiş; diğer ikisinde orijinal genel yardımcıların
uygulamanın gözlenen çağrısında seçilmeyen dalları import üretiyor.

| API | Orijinaldeki yer | Güncel kaynak ve sonuç |
|---|---|---|
| GetFileInformationByHandle | FUN_18004895c dosya-kimliği yardımcısı; FUN_180048d20 genel kopyalama yardımcısının bazı seçeneklerinden çağrılır. | Avatar çağrı noktasında 0x180031b4c MOV R8D,2; 0x180031b52 CALL048d20 gözlenir. Mode2 doğrudan CopyFileW(...,FALSE) dalına girer ve dosya-kimliği yardımcısını çağırmaz. Bizde content_handlers arayüzünün copy_overwrite sözleşmesi ve Environment adaptörü aynı dalı uygular. Tam çok seçenekli std helper gövdesi linklenmediği için bu import gerekmez. Gözlenen bu kullanımda aktarım eksiği bulunmadı. |
| InitializeCriticalSectionAndSpinCount | Eski Asio helper027be0; ayrıca eski VCRT/UCRT compatibility thunk'ları050c38 ve6707c. | RE native adaptörü include/preloader_l/net/transport_service_constructor_native.hpp içinde hâlâ var. Aktif ağ yolu src/net/asio_service.cpp ile yeni vendored Asio'dur; win_mutex SRWLOCK kullanır. İncelenen yeni statik UCRT de InitializeCriticalSectionEx import eder, eski SpinCount fallback importunu etmez. Eski native adaptörün her gövdesi Production'da canlı kalmak zorunda değildir. |
| TryAcquireSRWLockExclusive | Genel eski mutex helper04a6d4'ün try/timed dalları. | Gözlenen doğrudan çağırıcı04a6ac, XOR EDX,EDX ile deadline=nullptr verir. Bu yol blocking AcquireSRWLockExclusive kullanır. Sağlanan anadius-execution-source da açıkça yalnız bu süresiz yolu kurtarır; güncel execution.cpp aynı sözleşmeyi taşır. İncelenen MSVC14.51 _Mtx_lock gövdesinde blocking Acquire vardır; Try API yalnız try/timed gövdelerinde görülür. Yeni Asio'da try_lock tanımı da mevcuttur ama aktif kullanım yoksa import üretmez. Gözlenen uygulama yolunda kayıp try-lock çağrısı bulunmadı. |
| VerifyVersionInfoA | Eski IOCP constructor024990, işletim sistemi sürümüne göre bekleme davranışı seçer. | RE native adaptörü include/preloader_l/net/transport_iocp_constructor_native.hpp içinde vardır. Aktif yeni Asio get_gqcs_timeout, _WIN32_WINNT>=0x0600 için doğrudan INFINITE döndürür. Brownfield'den alınan0x0601 hedefi bu yolu seçtirir. Uygulamadaki ayrı VerifyVersionInfoW host/hook kontrolü farklıdır. |

Dolayısıyla yalnız bir Asio farkı yoktur: kilit tipi ve eski sürüm kontrolü iki
Asio değişikliğidir; critical-section için modern CRT'nin kaldırılan compatibility
fallback'i de ayrıca önemlidir. Try-lock ve dosya-kimliği örnekleri ise genel
yardımcı gövdesinin binary'de bulunması ile uygulamanın o dalı kullanmasının
farkını gösterir. Bu sonuç, tüm DLL için tam davranış eşdeğerliği iddiası değildir.

## CreateFile2: somut kaynak

Kullanıcı logu MSVC19.51.36260 ve Tools/MSVC/14.51.36231 yolunu gösteriyor.
Microsoft'un yayınladığı14.51 CRT x64 Desktop paketini indirdik. Paket SHA256'sı
URL/metadata pin'iyle eşleşti; içindeki libcpmt.lib yolu14.51.36231. Bu, kullanıcının
yerel LIB hash'iyle karşılaştırılmış olduğu anlamına gelmez.

libcpmt.lib(filesystem.obj) içinde __std_fs_remove ve __std_fs_open_handle gibi
gövdelerde __imp_CreateFile2 relocation'ı doğrudan gözlendi. Uygulamanın fs::remove
yolu bu standart filesystem ailesini kullanır. İzole __std_fs_remove bağlantısı
CreateFile2 importunu tekrar üretti. Bunu first-party bir CreateFile2 çağrısı
veya Gulrak kalıntısı açıklamıyor; modern MSVC STL'nin dosya açma backend'i açıklıyor.

Microsoft STL PR5434 (29 Nisan2025), eski desktop CreateFileW yolunun CreateFile2
ile değiştirilmesini ve bazı eski fallback'lerin kaldırılmasını belgeliyor.
Ancak bundan “modern MSVC'nin hiçbir yerinde GetFileInformationByHandle yok”
sonucu çıkarılamaz: incelediğimiz14.51 arşivinde ilgili yardımcı hâlâ mevcut.
Bu nedenle o API'nin üründe yokluğunun burada doğrulanan açıklaması genel kopyalama
yardımcısı yerine gözlenen mode2 dalının bağlanmasıdır.

Asio için _WIN32_WINNT=0x0601 tanımlamak önceden derlenmiş libcpmt.lib'i yeniden
derlemez ve bu API seçimini geri çevirmez. CreateFile2 Windows8 ile geldiği için
yalnız makro veya PE başlığına bakarak Windows7 çalışma uyumluluğu iddia edilemez.

## HeapQueryInformation: Asio aligned allocation üzerinden CRT bağlantısı

Vendored Asio memory.hpp'nin MSVC dalı aligned_new/aligned_delete için
_aligned_malloc/_aligned_free çağırır. İncelenen resmi Windows SDK26100.7627
statik libucrt.lib arşivinde:

1. align.obj, aligned allocation/free ve aligned reallocation gövdelerini aynı
   non-COMDAT .text$mn bölümünde barındırır.
2. Aynı bölümün reallocation dalı _expand'e relocation içerir.
3. expand.obj, _expand/_expand_base gövdelerini ve __imp_HeapQueryInformation
   relocation'ını içerir.
4. Microsoft'un resmi CRTSource22621.3 kaynağındaki expand.cpp, küçük heap bloğunu
   küçültme başarısız olduğunda LFH olup olmadığını HeapCompatibilityInformation
   ile sorgular. Bu sorgu heap ayarını değiştirmez.

İzole _aligned_malloc/_aligned_free bağlantısının logu align.obj ardından
_expand için expand.obj yüklendiğini gösterir ve final importta HeapQueryInformation
görülür. Aynı CRT ile sıradan malloc/free deneyinde import yoktur. Yani yalnız
“CRT olabilir” düzeyinde kalmadık: bu kütüphane setinde importun nasıl doğduğunu
tekrar ürettik. Aligned reallocation'ın uygulamada çalışması gerekmeksizin,
aynı nesne/bölümdeki bağlayıcı bağımlılığı bu importu getirebilir.

## Yedi kontrollü bağlantı deneyi

| Deney | Sonuç |
|---|---|
| malloc/free | HeapQueryInformation yok. |
| _aligned_malloc/_aligned_free | HeapQueryInformation var. |
| _Mtx_lock | AcquireSRWLockExclusive var, TryAcquireSRWLockExclusive yok. |
| _Mtx_trylock | TryAcquireSRWLockExclusive var. |
| __std_fs_remove | CreateFile2 var; GetFileInformationByHandle yok. |
| CopyFileW(...,FALSE) | CopyFileW var; GetFileInformationByHandle yok. |
| Genel __std_fs_copy_file(...,2) | Kullanılmayan seçenek dalları da bağlı olduğu için GetFileInformationByHandle var. |

Her deneyde gerçek resmi statik CRT/STL arşivleri kullanıldı. Derleyici Clang21
MSVC COFF hedefi, bağlayıcı LLD idi; native cl.exe/link.exe çalıştırılmış gibi
sunulmuyor. /O2 /MT /GS, açık statik kütüphane girdileri ve /OPT:REF kullanıldı.
Probe /Zl yalnız varsayılan kütüphane direktiflerini kaldırır; CRT kütüphaneleri
açıkça verilir. Hiçbir probe'da unresolved symbol zorlaması veya sahte API stub'u
kullanılmadı. Yedi DLL de hatasız bağlandı; import tabloları okunarak beklentiler
7/7 doğrulandı. Bunlar çalıştırılmadı; SURGE Golden veya oyun kabulü değildir.

## Kapsam ve kalan kesinlik sınırı

Kullanıcının karşılaştırdığı final MSVC DLL/MAP bu turda sağlanmadı. Bu nedenle
hedef DLL'deki her call-site RVA'sını kesin eşledik denmiyor. Bununla birlikte
RE çağrı argümanları, mevcut adaptörler, gerçek MSVC kütüphane nesneleri ve
karşılaştırmalı bağlantı deneyleriyle farkların somut oluşma yolları bulundu.
Ürün kodu, mevcut ZIP'ler, vendor içerikleri, export/ordinal/Golden politikaları
değiştirilmedi; yeni bir işlev kaybı tespit edilmiş gibi import eklenmedi.

İndirme notu: VS channel manifestinin beyan edilen SHA256/uzunluğu ile dönen
manifest byte'ları eşleşmedi; bu durum kayda alındı. Manifest yalnız resmi payload
URL'sini keşfetmek için kullanıldı. İncelenen CRT payload'unun kendi SHA256'sı
pin'le eşleşti; metadata uzunluğu farklıydı ve hash yerine boyuta güvenilmedi.
Yerel kullanıcı kütüphanesiyle byte eşitliği ayrıca doğrulanmış değildir.

## Birincil kaynaklar

- [Microsoft STL PR5434](https://github.com/microsoft/STL/pull/5434)
- [Microsoft STL filesystem kaynağı, sabit commit](https://github.com/microsoft/STL/blob/b6f01e438b68ee497f9c08313e49d12bb2f0d8e0/stl/src/filesystem.cpp)
- [Microsoft STL mutex kaynağı, sabit commit](https://github.com/microsoft/STL/blob/b6f01e438b68ee497f9c08313e49d12bb2f0d8e0/stl/src/mutex.cpp)
- [Microsoft CRTSource22621.3 paketi](https://www.nuget.org/packages/Microsoft.Windows.SDK.CRTSource/10.0.22621.3)
- [Microsoft SDK CPP x64 26100.7627 paketi](https://www.nuget.org/packages/Microsoft.Windows.SDK.CPP.x64/10.0.26100.7627)
- [Microsoft _expand açıklaması](https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/expand)
- [Microsoft HeapQueryInformation açıklaması](https://learn.microsoft.com/en-us/windows/win32/api/heapapi/nf-heapapi-heapqueryinformation)

Tam import sonuçları LINK_PROBE_RESULTS.json, çağrı kanıtları RE_CALL_PATHS.json,
kütüphane/object hash'leri ilgili JSON kayıtlarında. Probes altındaki .c/.map/log
dosyaları tanısal kanıttır; Microsoft kütüphaneleri ve DLL binary'leri bu rapor
paketine dahil değildir.

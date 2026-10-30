> Güncel kopyalama yardımcısı: [RE_COPY_RECOVERY.md](RE_COPY_RECOVERY.md).
> Ayrıntılı kütüphane/link araştırması: [API_IMPORT_RESEARCH.md](API_IMPORT_RESEARCH.md).

# Kullanıcının son import karşılaştırmasının değerlendirmesi

Girdi: kullanıcının Anadius ve güncel MSVC DLL için verdiği 11 API farkı.
Bu karşılaştırmaya yeni bir native DLL veya linker MAP dosyası eklenmedi.
Aşağıdaki kaynak kanıtı ile binary düzeyinde kesin çağıran tespiti ayrıdır.
Importun varlığı çağrının normal başlangıçta çalıştığını, yokluğu da ilgili
yüksek seviyeli işlevin eksik olduğunu tek başına göstermez. Verilen yüksek/
orta/düşük öncelikler bu tablodan doğrulanamaz.

| API | Kaynak kanıtı ve sonuç |
|---|---|
| GetFileInformationByHandle | Genel048d20 kopyalama gövdesi ve04895c kimlik fallback’i orijinal dökümden yeniden oluşturuldu. Avatar artık tam gövdeyi mode2 ile çağırır. Legacy API gerçek native helper içinde kullanılır ve güncel Release geliştirme DLL’inde statik import doğrulandı; native MSVC tekrarı ayrıdır. |
| InitializeCriticalSectionAndSpinCount | Eski RE servis mutex constructor'ında var. Ürünün aktif transport'u kullanıcının yeni Asio sürümünü kullanır; vendored win_mutex SRWLock başlatır. RE constructor kaynakları karşılaştırma/test grafiğinde bulunabilir; her kaynak gövdesi uygulama DLL'ine canlı olarak bağlanmaz. |
| TryAcquireSRWLockExclusive | Orijinal FUN_18004a6d4 mutex yardımcı gövdesinin sıfır/zamanlı bekleme dallarında var. Yeni Asio win_mutex::try_lock içinde de var; kullanılmayan inline/CRT dallarının import üretmemesi mümkündür. Yokluğu kilitlemenin eksik olduğunun kanıtı değildir. |
| VerifyVersionInfoA | Eski IOCP'nin çalışma zamanında sürüm kontrolü. Yeni Asio get_gqcs_timeout, _WIN32_WINNT>=0x0600 olduğunda bu kontrolü derlemez. Brownfield'den alınan0x0601 hedefi bu koşulu sağlar. Uygulamanın ayrı VerifyVersionInfoW hook/setup kontrolü farklı bir yoldur. |
| CreateFile2 | MSVC14.51 libcpmt.lib(filesystem.obj) __std_fs_remove/__std_fs_open_handle gövdelerinde doğrudan relocation bulundu; izole native kütüphane bağlantısı importu yeniden üretti. Güncel standart filesystem silme yolu bu aileyi kullanır. |
| GetExitCodeThread | MSVC STL cthread.cpp _Thrd_join referansında bulunur. std::thread join/CRT bağımlılığıyla uyumludur; native DLL için kesin çağıran eşlemesi ayrıca gerekir. |
| HeapQueryInformation | Asio’nun MSVC aligned allocation/free yolu UCRT align.obj üzerinden _expand/expand.obj bağımlılığını getirir. Resmi statik kütüphaneyle aligned allocation deneyinde import var; sıradan malloc/free kontrolünde yok. Kullanıcının final DLL call-site haritası ayrıca incelenmiş değildir. |
| InitializeSRWLock | Kullanıcının vendored Asio'sunda detail/win_mutex.hpp constructor'ında doğrudan bulunur. |
| RaiseFailFastException | src/runtime/application.cpp HookErrors::callback_exception doğrudan çağırır. Sağlanan Bluefield src/windows/hooks.cpp aynı hata/log/fail-fast/abort sırasına sahiptir. Normal başlangıç çağrısı değildir; hook callback'inden beklenmedik exception çıkarsa çalışır. Anadius'a göre uygulama farkıdır; seçilen Bluefield hook yaşam döngüsünden sapma değildir. |
| ReadProcessMemory | src/platform/crypt_query_host.cpp, mevcut işlemin UTF16 dosya adını güvenli okur; geçersiz bellek okunamazsa selector fallback yapar. Kullanıcının seçtiği Bluefield 11. hook korumalı okuyucusundan gelir. |
| WaitForSingleObjectEx | MSVC STL _Thrd_join referansı INFINITE ve alertable=FALSE ile çağırır. Ex adının bulunması alertable bekleme yapıldığı anlamına gelmez. |

İlk karşılaştırma tek başına yeni API ekleyip kaldırmıyordu; sonraki kullanıcı talimatıyla tam kopyalama gövdesi yukarıdaki güncel kayıtta yeniden oluşturulmuştur. Sadece listeleri eşitlemek için
kullanılmayan import üretmek davranış eşdeğerliği sağlamaz. Kullanıcı tarafından
seçilmiş yeni vendor/standart CRT ve Bluefield hook bağımlılıkları korunur.
Somut davranış farkı ancak ilgili çağrı yolu ve hata durumlarıyla incelenebilir.

Kanıt yolları: src/platform/crypt_query_host.cpp; src/runtime/application.cpp;
src/platform/status_windows.cpp; third_party/asio/asio/include/asio/detail/
win_mutex.hpp ve impl/win_iocp_io_context.ipp. Orijinal RE ve Bluefield donor
dosyaları ürün ağacı dışındaki korunmuş girdilerdir. MSVC STL thread referansı
build/qa/MSVC_STL_cthread_reference.cpp altında tanısal kayıttır; kullanıcının
kurulu MSVC sürümünün tam binary haritası yerine geçmez.

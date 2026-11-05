# Attach hata prefix'leri ve poisoned thread admission düzeltmesi

Özgün senkron başlangıç politikası korunuyor. Bu aşamada `tracked_threads.cpp` içindeki bir admission denetimi düzeltildi, header sözleşmesi ve mevcut `backend_tests.cpp` senaryo kaynakları güncellendi. **Derleme, proje testi, link veya DLL çalıştırması yapılmadı.** Aşağıdaki sonuçlar statik kaynak incelemesidir; PASS iddiası yoktur.

## Somut kaynak açığı

`TrackingThreads::create_thread`, önceden yalnız `accepting` değerini denetliyordu. Lower create exception'ı veya başarılı create sonrasında başarısız/throwing duplicate, `poison=true` bırakıyor; `accepting` ise henüz stop çağrılmadığı için true kalabiliyordu. Sonraki create yeni bir ticket ayırıp lower provider'ı tekrar çağırıyordu.

Denetim artık aynı mutex altında `!accepting || poison` koşuluyla, ticket ayırmadan ve lower'a girmeden reddediyor. Bu, yeni source ownership adapter'ının düzeltmesidir; özgün native thread algoritmasının kurtarılan yeni bir gövdesi değildir. Zaten reserve edilmiş eşzamanlı create iptal edilmez. Önceki unknown ticket/handle/provider sahipliği korunur; poison temizlenmez, shutdown başarılı sayılmaz.

Kesin null create sonucu kendi ticket'ını kaldırır ve poison üretmez. Böyle bir çağrıdan sonra açık admission üzerinde yeni istek hâlâ kabul edilebilir. Burada bütün bootstrap'ın otomatik tekrar başlatıldığı veya eski payload'ın geri alındığı iddia edilmiyor.

Regresyon kaynakları `--poison`, `--duplicate-throws`, `--create-throws` durumlarında **stop_and_join çağrılmadan önce** ikinci create isteğinin reddini ve lower create/ticket sayılarının artmamasını bekler. Bu yerleşim önemlidir: stop_and_join zaten accepting=false yaptığı için sonrasındaki denetim eski açığı ayırt etmezdi. Mevcut throwing wait/close terminal senaryoları da kapalı admission beklentisini korur. Normal senaryoda bilinen null create sonrası tekrar kabul, original detach ve duplicate drain beklentisi eklendi. Bu senaryolar çalıştırılmadı.

## İlk worker öncesi ve sonrası ayrımı

| Prefix | Elde bulunabilen sahipler | Mevcut kaynak sonucu / sınırlama |
|---|---|---|
| Factory/Runtime construction, publication öncesi | Domain, provider'lar, configuration, Worker'ın controller backing'i; kısmi constructor sahipliği olabilir. | Native Binding henüz yayımlanmamış olabilir. Bunun tamamının loader altında güvenle geri alınabildiği kanıtlanmış değildir; mevcut factory/control sözleşmeleri loader dışıdır. |
| Runtime published, application henüz başlamadı | Native Binding ve kalıcı Errors bağlantısı. | Zero worker, yayımlanmış Binding'i otomatik kapatmaz. Constructed ve Bound phase'leri ayrı tutulur. |
| `038970` içindeki register_exit/install_terminate → environment/console/log/configuration | CRT'de thunk pozisyonları, yayımlanmış callback route'u, değiştirilmiş terminate handler ve configuration/IO prefix'leri; henüz worker olmayabilir. | register_exit sonucu özgün gövdede kullanılmaz. CallbackRegistry başarısız kayıtta callback öğesini kaldırsa da route yayımlanmış kalabilir. Tam rollback worker sayısından çıkarılamaz. |
| İlk payload allocation veya ilk known-null CreateThread | Payload/context/module-ref prefix'i; önceki callback kayıtları. | `05e8e0` normal null create yolunda Context/module cleanup çağrılarını yapar; bunların sonuçları ve bütün üst sahiplerin rollback'i aynı şey değildir. Bootstrap thread_error yoluna gider. |
| Lower create throws | Bir thread başlamış olabilir; geçerli dönen HANDLE yoktur. | Ticket unknown/untracked ve poison kalır. Yeni create artık reddedilir. Null dönüşten farklıdır. |
| Create başarılı, duplicate false/throws | Başlamış veya pending gerçek iş; join proof'u eksik/unknown. | Başarılı orijinal handle sonucu değiştirilmez; ticket ve owner tutulur. Yeni oluşturma reddedilir. Önceki çağrıya geri dönük cancellation veya join uydurulmaz. |
| İlk successful create/detach sonrası host kurulumu | En az ilk thread ticket'ı, olası callback/hook kurulumu; orijinal handle kapanmış olabilir. | Sonraki host veya ikinci create hatası first-worker öncesi rollback değildir. Native detach orijinal handle'ı kapatır; retained duplicate ayrı sahipliktir. |
| İki worker sonrası senkron main library yüklemesi | İki worker ve hook/Online prefix'leri; ek loaded module handle'ları oluşabilir. | LoadLibrary/conversion/UI hatası öncesi etkiler kaybolmaz. Loader altında worker join eklenmedi. |
| Üçüncü create veya son guard_return hatası | Önceki bütün prefix'ler ve olası üçüncü thread. | Henüz application_entry dönmemiş olsa da pending/suspended thread'ler tutulmalıdır. Native active-call sayısı onların fiziksel çıkışını kanıtlamaz. |

Mevcut aday DllMain, önce `require_loader_attach_runtime` factory'sini, sonra dönen Runtime'ın `application_entry` fonksiyonunu çağırır. Bu iki evre karıştırılmamalı: factory'nin catch/rollback'i daha sonra gerçekleşen application startup hatalarını kapsamaz. Native `application_entry` noexcept boundary'si exception'ı `Errors::callback_exception` politikasına aktarır ve geri dönmez; mevcut kalıcı abort politikası tam rollback veya başarılı FALSE attach dönüşü değildir.

RuntimeOwner.close, admitted application call devam ederken önce application admission'ını pause eder ve false döner; backend stop/join'e geçmez. Bu, reentrant cleanup'ın startup'ın kalan provider/worker çağrılarını yarıda serbest bırakmasını önler. Bu kontrolü bypass eden bir loader rollback yolu eklenmedi.

## Kalan iş

İki `require_loader_*` tanımı, senkron loader profili ve tam attach rollback/detach sözleşmeleri hâlâ açık. Bu düzeltme yalnız tracker'ın belirsizlik sonrası yeni thread kabulünü kapatır; bütün provider işlemlerini ilk poison anında durdurmaz ve daha önce reserve edilmiş işleri iptal etmez. Özgün DLL'ler ve ZIP değiştirilmedi. Toplam DLL için ölçülmüş yüzde artışı yoktur.

Güncel kaynak satırları, hashler, önceki kanıt bağlantıları ve statik inceleme sınırlamaları: `attach-failure-prefix.evidence.json`.

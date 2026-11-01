# FIX27 — Ticket yazıcısının initial-open false dalı geri bağlandı

Tam kaynak/build/QA devam paketi, NON_RELEASE; exact FIX26 ebeveyni korunur. Kaynak kullanıcının RE C++ paketidir. Yeni runtime kapsamı open-phase I/O booleanı ve aynı atomik stage ownerının aşamalı kullanımıdır.

## Kaynak farkı ve FIX26 tablosundaki bağlama düzeltmesi

FIX26 LIC11 satırı LicenseEffects.write_ticket metodunu aktif yazıcı gibi gösteriyordu. Bu bağlama eksikti: Application licensing handlera LicenseFiles wrapperını verir; onun write_ticket overrideı write_binary_03c1d0 ve NativeFactoryyi kullanır. Doğrudan LicenseEffects metodu bu yolda çağrılmaz. Yeni testin kalan Effects.write_ticket metodu çağrılırsa özellikle hata vermesi gerçek routeu denetler. Güncel18 satırlı tablo doğru caller/line/hashleri taşır.

Kaynak write_binary yalnız initial_state!=0 halinde false döner. Paket NativeOutput constructorında gerçek ofstream açılır ve başlangıç durumu okunur. Aday NativeOutput ise hep initial_state0 bildiriyor, dosya açmayı close/atomic_writea erteliyordu. Bu yüzden ordinary open başarısızlığı kaynak false/fallback dalına girmeden exceptionla çıkıyordu. Geç write/flush/close hatalarını da topluca falsea çevirmek kaynak boolean sınırını genişletirdi.

## Yapılan değişiklik

NativeOutput constructorı exclusive sibling stagei gerçekten açar. Yalnız bu setup sırasında std::system_error (filesystem_error dahil) initial failbit2 olur. Değişmeyen write_binary false döndürür; ticket fallback dialogu ve mevcut exit(-1) Effecti çalışır. Source Wine predicate sırası, blocked flag ve token-attempt değerleri korunur.

Var olan Stage ownerı open/write/finish/publish metotlarına ayrıldı. Normal atomic_write/atomic_batch tam Stage constructorını sürdürür. Yeni AtomicFileWriter aynı RAII/OS kodunu stream boyunca tutar; target commit öncesi değiştirilmez. Stream stage payloadı doğrudan private temporaryye yazar; eski tüm-payload memory bufferı yoktur. Yeni streamde stage/handle ömrü open→close arasındadır; bu zamanlama/allocation footprint değişimi açıkça kayıtlıdır. Global publication/batch mutexi kalır; ayrı caller metotları arasında kilit tutulmaz.

Post-open write/flush/commit hataları exception olarak kalır. Allocation, recovery, ownership/domain ve mevcut staging retry-budget hataları initial falsea çevrilmez. Ownership/layout,100 staging retry değeri, default/profile/avatar/CFG, source exit kodları ve process/service/session politikası değişmedi. Pristine vendor1060 dosya ve önceki51 C++ fixture aynıdır. Atomic staging user requirementıdır; orijinal ofstream share/permission/exception-mask/native ABI eşdeğerliği denmez.

Shared NativeFactory outputu standalone ini/persistence callerlarında da kullanılabilir; bu kapsam gizlenmedi. Aktif Application achievement writerı atomic_batch overrideını, online profile writerı ayrı atomic_write yolunu kullanır. Genel binary writer, LicenseFiles ve license handler bodyları değiştirilmedi.

## Kanıt ve açık durum

Test gerçek NativeFactory→LicenseFiles→Handler yolunda kontrollü stage-open EACCES üretir. Eski core exceptionla fallbacki atlar; yeni kod false-result Wine/normal fallback dialogunu ve exit(-1) Effectini çalıştırır. Gerçek setup NotDirectory, başarılı dosya, abandoned stream, write/flush/commit hatası, allocation/recovery sentinel, unchanged budget ve caught-write sonrası poisoned-stage commit kontrol edilir. Hatalarda eski target byteı ve geçici dosya temizliği denetlenir. Interposition yalnız Linux test executableındadır; production fault selector yoktur. UI/exit sentetiktir; oyun veya native DLL sonucu değildir.

D14.LIC11.initial_open_failure düzeltildi. D14ün license process-vs-session, host CRT ve retained late atomic-error farkları açık kalır. Whole finding9→9; sahte closure yapılmaz. Windows/MSVC/file-sharing/permission/replace/provider/CRT/game ve actual DLL Golden NOT_RUN. Final sonuçlar QA raporunda verilmiştir.

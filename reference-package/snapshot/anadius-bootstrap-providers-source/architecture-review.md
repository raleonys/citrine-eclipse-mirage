# B5 bağımsız kaynak incelemesi

İnceleyen: `/root/b5_source_review`. Sonuç: **PASS**, aşağıdaki açık kullanım koşulları altında üretim kodunda engel bulunmadı. Dört üretim dosyasının tam SHA-256 değerleri ve on beş bulgu `architecture-review.json` içindedir. Önceki B4 ZIP kaynak snapshot'ındaki **710 dosya bağımsız olarak karşılaştırıldı; hepsi değişmemiştir**.

Configuration, 21 bağımlılık metodunun yedisini seçilmiş mevcut kaynak/CRT/SDK yollarına bağlar. Kalan on dört politika açık alt sağlayıcıda kalır. Dosya sistemi code-page seçimi mevcut encoding gövdesinden gelir; bilinmeyen narrow/UTF-8/path-resolution davranışı tahmin edilmez. Compiler `tolower` için geçersiz signed-byte değerler C kütüphanesi UB'sine ulaşmadan reddedilir; özgün locale/multibyte eşdeğerliği iddiası yoktur.

CompilerConsole, sabit konumlu üç `FILE*` slotunu opak token olarak sunar. `freopen_s` ayrı bir `FILE*` output hücresi alır; özgün MSVC `FILE` record'u kopyalanmaz veya alias edilmez. Kaynak/output token'ı farklı veya yabancıysa çağrı reddedilir. Başarısız reopen sonrasında null kalan slot tekrar kullanılamaz. Bu açıkça seçilmiş yeniden derleme politikasıdır; özgün `0x58` byte FILE ABI'sini kapatmaz.

NativeThreads, her iş için provider ve Context pointer'larını taşıyan ayrı bir Launch zarfı kurar. Callback zarfı kurtarılmış `05e7a8` gövdesine girmeden önce siler; `ExitThread` sonrasında C++ destructor çalışmasına güvenmez. Başarılı create dönüşünden sonra creator, zarfı veya Context'i tekrar okumaz. Normal başarısız create zarfı siler, hata kodunu korur ve caller ID'sini değiştirmez. Worker sonucu bilinmeden exception çıkarsa zarf tutulur; B3 poison durumu güvenli kapanışı reddeder.

Factory, mevcut somut `crt_glue::ThreadEffects` sınıfını seçer ve çağıranın verdiği CRT domain'ini ortak sahiplik ile canlı tutar. Bütün pending/suspended worker'lar ve FLS exit temizliği provider yıkımından önce bitmelidir. B3/B4 kontrollü kapanış bu sırayı sağlar; ham NativeThreads destructor'ı kendiliğinden join yapmaz.

Test kaynakları incelendi. Parent'ın bildirdiği hızlı sonuç 53 çekirdek ve 110 entegrasyon kontrolüdür; bu inceleyen bağımsız test veya tam ortak build çalıştırdığını iddia etmez. Entegrasyon üç bize ait Win32 worker'ı sentetik join sırasında sırayla başlatır. Test sayacı kontrolleri resume öncesi veya wait sonrası yapılır; aynı non-atomic sayaç eşzamanlı değiştirilmez. Dosya girdileri bellek içidir, console ve encoding etkileri enjekte edilir. Tam gerçek CRT ThreadDomain/FLS graph'ı bu yeni entegrasyon testinde çalıştırılmış sayılmaz.

LastError bookkeeping, gerçek Win32 thread hata alanını kullanır; enjekte edilen create ve alt sağlayıcı `last_error` yolu aynı alanı kullanmalıdır. Kaynak sağlayıcıların kendi ömür/senkronizasyon sözleşmeleri zorunludur. Genel native concurrency, foreign CRT/STL/EH/SEH/FILE ABI, loader lock, image unload ve PE girişi açık kalır. Yeni native gövde ve kapanış **0**; tam DLL hazırlığı **BLOCKED**.

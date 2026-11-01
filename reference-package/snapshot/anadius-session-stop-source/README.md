# B50 — Receive/send ve mevcut oturumların kapanışı

Mevcut `Stop`, açıkça seçilen `Scope::SessionIo` modunda receive/send ve owned Posted işlerini kapsar. Varsayılan `AcceptOnly` korunur. Yeni production birimi eklenmedi; değişiklikler önceki `stop.hpp/.cpp` ve `io_memory.hpp/.cpp` dosyalarındadır. Bu klasördeki iki yeni C++ dosyası testtir.

Typed SDK sarmalayıcıları socket, private OVERLAPPED/operation, buffer aralığı ve kaynak control sahipliğini çağrıdan önce kaydeder. Dönüşte 0 veya hata997 Kernel, diğer bilinen hata Software olur; eski iki argümanlı arm Unclassified kalır. Public binding private projection'ın yerine geçemez. SDK buffer/owner/binding değiştirirse veya sonuç belirsizse sahiplik korunur. SessionIo böyle bir işlemi tahmin ederek iptal etmez.

Controller'ın bağımsız, thread-safe kanıtı yeni application/foreign/rawjob admission'ını durdurmalı ve raw/deferred işleri bitirmiş olmalıdır. Mevcut izlenen receive/send ve oturum sahipleri bu aşamada canlı olabilir. Controller yalnız NULL-OVERLAPPED wake yollar; gerçek CancelIoEx özgün worker üzerinde tam SDK socket/OV ile çağrılır. İptalin dönüş değeri finality değildir. Gerçek physical key0/status/bytes/OV paketi, generation/submission eşleşmesi, native owner0 kaynak temizliği ve başarılı after/reap dönüşü gerekir. Posted, native flag0 ile temizlenir; CancelIoEx veya kernel paket kanıtı sayılmaz. ACK sırasında eski operation/control dereference edilmez. Private aktif Posted kaydı public binding temizlenerek gizlenemez; yalnız actual OwnedAccept ile aynı slot ve tam operation/OV eşleşmesi olan inert rezervasyon atlanır.

Çekirdek testler gerçek Parent, native023000 control, IOCP ve yerel named-pipe ReadFile/WriteFile kullanır; scheduling sınırı açıkça elle sürülür. Entegrasyon actual SourceBackend/Runtime ve üç gerçek Domain thread'i üzerinden native-created oturuma ulaşır. Aynı anda pending accept/read/write bulunur. İlk ve ara paketlerden sonra kalan buffer pin/control/socket korunur; son kaynak tamamlanmasından sonra gerçek control disposal ve Profile reap doğrulanır. Testler gerçek ağ veya WSARecv/WSASend/AcceptEx interoperability kanıtı değildir. Özgün DLL/oyun çalıştırılmaz ve değiştirilmez.

Tam DLL henüz üretilmedi. Dış admission/rawjob/foreign kanıtı, tüm Resources bileşimi, erken startup hata yolları ve loader attach/detach bağlantıları tamamlanmalıdır. Yeni native gövde veya ABI kapanışı yoktur; test sayıları toplam DLL tamamlanma yüzdesine çevrilmez.

[Kanıt](evidence.json) · [Bağımsız inceleme](architecture-review.md) · [Sınırlar](resources-audit.json)

## B50 ortak doğrulama

**213 çekirdek + 139 entegrasyon = 352 kontrol PASS.** Ortak build 271 nesne (267 C++ / 4 C), 329 normal test grubu ve 19 framework terminal senaryosu içerir. Önceki 926 kaynak aynı; 4 önceki dosya açık hash kaydıyla güncellendi; toplam 932 kaynak. Yeni typed normal gövde 0, yeni native ABI/provider kapanışı0; 501 adres, 505 karşılaştırılabilir derlenmiş etiket ve52tarihsel kapanış. Bileşim ile raw-all defterleri ayrıdır. Gerçek alt sağlayıcılar, foreign ABI, image unload ve PE girişi açık kalır.

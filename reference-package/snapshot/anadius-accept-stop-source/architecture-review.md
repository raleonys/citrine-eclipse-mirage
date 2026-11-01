# B49 bağımsız mimari inceleme

**PASS: seçili kaynak kapsamı. Üretim DLL hazırlığı BLOCKED.**

Üretim/core/integration yazarlarından bağımsız statik inceleme tamamlandı. İnceleyici EXE/test/build/probe/mutasyon yardımcısı çalıştırmadı. Ortak build inceleme anında PENDING; sonraki ortak doğrulama ayrı kayıttır.

Sekiz kaynak hash'i donmuş kayıtla aynı. B48 ZIP içindeki926 ön-imaj doğrulandı:922aynı,4değişik,4yeni; toplam930kaynak. Kullanılan değişmez archive ZIP üyesiyle aynı.

Controller yalnız özel NULL-OVERLAPPED wake gönderir. Gerçek CancelIoEx özgün worker/fiber/PTD ve Parent kapsamında exact listener/OV ile çalışır. TRUE/FALSE/ERROR_NOT_FOUND finality değildir. Kernel key0/OV/status/bytes ve generation+submission doğrulanır; kaynak0271a0 tamamlanmadan ACK verilmez. Durmuş gerçek başarı da owner0 ile düşürülür. Sahte terminal packet veya scheduler work decrement yoktur.

Normal route lease'i owner seçimi, kaynak tamamlanması ve after/reap dönüşünü kapsar. Harici proof ile çakışan aktif/bitmiş route epoch'u isteği reddeder; yeni proof gerekir. Normal history büyümez. Self-pin Busy temizliğinden sonra bırakılır; Retiring provider callback'leri canlı metadata üzerinde mühürlenir. Geç-Requested testi gerçek normal pipe başarısı sonrası worker_returned kapısında istek gönderir; yeni get olmadan kapanır. Port-close spy hâlâ kuyrukta olan özel wake'i gözler; queue-empty iddiası yoktur.

Salt okunur doğrulanan yazar kanıtları:

- `/root/acceptor_tests`:317ana kontrol +9ayrı kesin-exit child901–909;20gerçek seçili kaynak hash'i.
- `/root/b22_integration`:162ana kontrol,0child;10gerçek kullanılan/11izlenen kaynak. Core test hash'i yalnız metadata. Değişmez V3/main JSON ve log birebir aynıdır.
- Toplam479ana kontrol. Root yalnız normalize etti; inceleyici yürütmedi. Değişmeyen resources.cpp, graph.cpp, worker.cpp yeni Runtime Dependencies ABI'si için taze derlenmiş bağımlılıklardır.

Integration gerçek RuntimeOwner/SourceBackend üç NativeThreads/Graph yolunda kendi overlapped named-pipe ReadFile→gerçek IOCP→default CancelIoEx995 ve gerçek başarı/ERROR_NOT_FOUND yarışını kullanır. Buffer sahipliği gözlenen packet ve gerçek kaynak emekliliğine kadar korunur. Bu gerçek AcceptEx/WSASOCKET interoperability veya ağ kanıtı değildir.

Receive/send/session/rawjob/foreign işler için bağımsız thread-safe durdurma/drain proof zorunludur. Bilinmeyen sonuç poison+retain olur. Native EH rollback, genel concurrency/kernel ABA/packet provenance, full Resources ve loader closure kanıtlanmaz. Erken-firstReady hata sınırları sürer.

Native/ABI delta0;501mapped/505eligible/139normal/52tarihsel closure; son native body B38. Common hedefi271object(267CPP+4C),327normal+19terminal;111composition dosyası/55object;98compiler observation. Bu sayılar common PASS iddiası değildir.

Registration preview ve beş uygulanmayan yardımcının yazarı inceleyicidir; root strict review sonrası uyguladı. Production/test review bağımsızdır; pipeline yazarlığı bağımsızlığı iddia edilmez. Metadata/paketleme root sorumluluğundadır. Tam hash ve kayıt yolları eş JSON içindedir.

İlk common çalışması legacy runtime_tests.cpp125 aggregate derlemesinde yeni optional member için missing-field Werror ile durdu; PASS değildir. Tek kaynak değişikliği `accept_stop;`→`accept_stop{};` açık boş varsayılanıdır; layout/değer semantiği değişmez. Eski sekizli proof/review snapshot korunmuştur. Güncel core final2(317+9child) ve integration V3(162/0child) yeni header ile tekrar yürütülmüş yazar kayıtlarıdır; inceleyici yürütmemiştir. Güncel common log `work/b49-common-v2-build.log`; inceleme anında PENDING.

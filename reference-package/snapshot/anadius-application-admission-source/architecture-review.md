# B51 bağımsız mimari inceleme

**PASS: seçili kaynak kapsamı. Üretim DLL hazırlığı BLOCKED.**

Üretim/core/integration yazarlarından bağımsız statik inceleme tamamlandı. İnceleyici EXE/test/build/probe/mutasyon yardımcısı çalıştırmadı. Common inceleme anında PENDING.

Altı hash freeze ile aynı: üç production değişikliği, bir mevcut owner regression testi ve iki yeni test. B50 ZIP932ön-imajı doğrulandı:928aynı,4değişik,+2test=934kaynak. Yeni production birimi yok; kullanılan archive ZIP üyesiyle aynı.

Binding önce canlı owner kontrolünü, sonra Application mührünü aynı gate altında yapar. Mühürlü fakat bağlı cached giriş FALSE döner; Bootstrap/provider/Errors çağrısı ve yeni source lease yoktur, LastError korunur. Bağlı olmayan/kapanmış hata politikası değişmez. Kabul edilen Application toplam ve ayrı sayaçları artırır; normal dönüş/exception unwind bırakır. Worker/Exit/Terminate yolları mühürden etkilenmez.

RuntimeOwner.close Stopping ardından backend stop/join öncesi pause çağırır. Eski source startup sürüyorsa false döner, provider/Binding korunur; eski başlangıç kalan worker işini tamamlar. Retry gerçek backend shutdown yoluna gider. Mevcut owner testindeki nested-close beklentisi yalnız bu amaçla Busy yerine false/stop0 olarak güncellenmiştir; sonraki retry/drain korunur.

Önemli sınır: sayaç yalnız Bootstrap SOURCE invocation içindir. Kalıcı Errors callback özgün invoke tasarımında lease bırakıldıktan sonra çalışır. Opaque Errors, reddedilen thunk epilogue, tüm machine-entry süresi ve image unload bu sayaçla kanıtlanmaz. Kalıcı Errors/code/backing ömrü ve dış quiescence gerekir.

Doğrulanan yazar kayıtları:

- Core /root/acceptor_tests:72ana kontrol +4ayrı kesin-exit child1101–1104;7gerçek seçili kaynak hash’i. Gerçek Bootstrap/native entry, kontrollü dependency/queued-worker ve kendi std-thread bariyerleri.
- Integration /root/b22_integration:96ana/0child;4gerçek kullanılan/6izlenen kaynak. Core ve değişen eski regression CPP metadata-only. DeğişmezV2/main JSON ve log aynı.
- Toplam168ana kontrol. Root yalnız normalize etti; inceleyici yürütmedi. Taze native_entries.cpp/owner.cpp değişmez B50 archive önünde derlenmiştir.

Integration actual RuntimeOwner/SourceBackend ve DÖRT gerçek aynı-Domain NativeThreads kullanır: eski Application caller +3Bootstrap worker. Eski Application source guard’da bloke iken close backend stop_sources/worker stop/cancel öncesi false olur. Cached tüm-reason girişler provider/Errors etkisi üretmez. Eski caller gerçek join sonrasında retry mevcut own-pipe CancelIoEx995/source cleanup, üç worker join, kayıtlı EXIT cleanup, FLS/controller kapanışını tamamlar. Alt Resources/SDK/policy sınırları açık seçilmiştir; bu full Resources veya gerçek network interoperability kanıtı değildir.

Başarısız duplicate Binding mührü açmaz. Başarıyla kurulan yeni Binding aynı static thunk’ı yeniden açabilir; cached pointer rebind/ABA ve unload çözülmüş sayılmaz. Yeni Application kapısı dış rawjob/deferred/foreign/hook admission ve loader-lock kontratını tamamlamaz.

Native/ABI delta0;501mapped/505eligible/139normal/52tarihsel closure, son bodyB38.271object(267CPP+4C),331normal+19terminal;111composition/55object ve98observation değişmedi. Common PASS iddiası yoktur.

Registration preview/beş hazırlık yardımcısının yazarı inceleyicidir; root uyguladı. Production/test review bağımsızdır; pipeline yazarlığı bağımsızlığı iddia edilmez. Root metadata/README/resources-audit salt okunur incelendi. Tam hash/kayıt yolları eş JSON içindedir.

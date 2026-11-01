## Profil ve kalan refactor

RF30'da surge.cfg örneğinin eksik dokuz User alanı düzeltildi;gerçek örnek
dosyayı okuyan regresyon bu tur da geçiyor. GetConfig normal34 servis kontrolü
geçer;legacy exit14 son unknown entity rejection beklentisi ile parser politikası
arasındaki farktır. Entity politikası veya diagnostic PASS olarak değiştirilmedi.
GetProfile index selector ve SDK handshake/session crypto incelemesi açıktır.
Oyundaki gerçek profil kök nedeni kanıtlanmadı;RFCL-005 OPEN kalır.

4 önceki source review kapalı,16 modül açık. Yeni mandatory import-refactor
kuralında 294 maintained code dosyası ve4 vendor integration PENDING,QUALIFIED0.
Bu dört eski review yeni kural için otomatik qualification değildir.
verify_source.cmd --require-refactor-complete bu nedenle beklenen exit1 verir;
normal local build pending kayıtlarla çalışabilir. M20.3'ün kaynak entegrasyonu
uygulandı;M20 kapanmadı. Sırada M20.4 Debug/native/CRT coverage vecomposition,
ardından B02 shutdown/bellek,B03 service/TCP/ASIO,B04 XML/zaman,B05 response,
B06 hooks/DLL,B07 son kaynak kapanışı var. Gerçek Windows layout/data kabulü
ve bütün semantik qualification ayrıca takip edilir.

GPL-3.0-only,preloader_l,surge.cfg,vendor pinleri,7 hook,35 binding veHF19/CURRENT
korunur. Governance normative mirror2026-10-21.1. Uygulama C++ değişmediğinden
sanitizerlar bu tur tekrarlanmadı. Bu kaynak teslimi NOT_RELEASE'tir.

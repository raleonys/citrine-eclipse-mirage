# B21 — Gerçek FLS/PTD izin kontrolü ve encoding sınırı

Callback izni artık yalnız thread/fiber metadata’sını değil, **gerçek domain FLS slotundaki PTD kimliğini** kontrol ediyor. Native admission, PTD attach sonrasında ve gövde/original erişiminden önce bunu doğrular. Seçili foreign factory ayrıca owned encoding sağlayıcısını her lower çağrı öncesi/sonrası aynı izinle sarar.

**25 çekirdek + 27 entegrasyon = 52 yeni kontrol PASS.** Bağımsız reviewer üretim/core ve entegrasyon yazarlarından ayrıdır; dokuz kaynak ve iki root-run EXE hash’i doğrulandı, reviewer rerun yok. Önceki 805 kaynağın 801’i aynı; dört dosya before/after hash kaydıyla değişti. Beş yeni dosyayla toplam 810 kaynak.

## İzin protokolü

CallbackThreads metadata mutex altında BodyReady/thread/fiber/poison durumunu doğrular ve probe rezervasyonu alır. Hem bu mutex hem dış ForeignDomain mutex’i bırakılır; actual FLS sağlayıcısı çağrılır. Sonra thread/fiber ve expected pointer doğrulanır. Null, sentinel veya başka pointer adopt edilmez, dereference edilmez ve kör onarılmaz. Yalnız kimliği doğru **fresh** PTD, ilk snapshot ile karşılaştırılır; errno/DOS alanı0x20–0x27 değişebilir. Borrowed PTD’nin içeriği okunmaz veya sahiplenilmez.

Getter throw, persistent identity/snapshot ihlali veya aynı owner’ın yakalanmış nested permission probe’u sticky poison ve retention üretir. Wrong-thread sorgusu owner’ı poison etmeden false döner. Probe sırasında metadata getter’ları başka bir kilide takılmadan çalışabilir. Bu checkpoint raw free, aynı adrese yeniden allocation, geçici değiştirip geri koyma veya arbitrary provider mutation’ı güvenli yapmaz.

**Cleanup başladığı anda gövde izni kalıcı biter.** İlk detach false dönse ve LastError0 olsa bile permission yeniden açılmaz. Bilinen detach/free/clear cleanup retry’si aynı thread/fiber üzerinde korunur; destructor/free tekrar edilmez. Unknown/poison durumlar için otomatik retry veya slot restore yoktur.

## Encoding sağlayıcısı

CheckedEncoding, owned `encoding::Calls` ve selected domain permission tutar. Beş virtual route’un her lower çağrısında ayrı pre/post kontrol yapılır; query/fill iki ayrı invocation’dır. Metadata kontrolü sırasında WinLastError korunur; lower sağlayıcının gerçek giriş/çıkış hata etkisi silinmez. Codepage/flags/pointer/count/capacity/used-default argümanları ve output prefixleri değiştirilmez.

Lower exception sonrasında PTD hâlâ geçerliyse aynı exception yeniden fırlatılır. Postcondition geçersizse ihlal önceliklidir; yazılmış prefix geri alınmaz, lower çağrı tekrarlanmaz. Factory bunu selected foreign Policy encoding yoluna otomatik bağlar. Doğrudan lower-provider reentry genel olarak engellenmiş değildir; dış sözleşme olarak kalır.

## Test ve kanıt

Core fresh/borrowed kimliği, errno/DOS değişikliği, wrong-thread ret, actual getter sırasında metadata erişimi ve hata kodu0 olan detach-false dahil cleanup retry’lerini doğrular. Altı own child null/sentinel/foreign slot, fresh snapshot ihlali, getter throw ve yakalanmış nested probe için active+poison+retention+LastError predicate’ini doğruladıktan sonra doğrudan TerminateProcess170–175 kullanır. Generic abort239 ve unexpected return230 başarı sayılmaz.

Entegrasyon gerçek foreign make_factory/native folder_a yolunda UTF8 locale/query/fill ve çağrılar arasında tam iki FLS probe’u doğrular. Başlangıç guard ihlalinde id7 doğrudan own original stub’a gidecek yoldur; original_calls **0** kalır. Provider locale/query/fill aşamasında slotu değiştirirse sonraki lower çağrı başlamaz; fill prefixi zaten yazıldıysa korunur. Valid lower exception, native wrapper cleanup tamamlandıktan sonra aynı exception identity ile Errors sınırına gelir. Typed state predicate’leri sonrası direct child kodları211–215; ilgisiz FLS/fixture abort PASS üretemez.

`source-evidence.json`, retained Ghidra VA/Evidence ID/line/hash kayıtlarını bağlar:0648fc/0649ec PTD constructor/destructor,064b4c cleanup,064d08 optional PTD,05d724 errno,048ac0 codepage,006e60 narrow,00fc00 folder_a,006d70 widen ve048b30 conversion. Bu checkpoint’ler yeni source tasarımıdır. B20 binary_session kaydı REA6.5.0/Ghidra12.1.4 kimliğini retained olarak taşır; B21’de target açılmadı, yeni original byte/decompile sorgusu yoktur. Original/rebuilt DLL/game çalıştırılmadı veya değiştirilmedi; user CFG/token/profile/achievement içeriği okunmadı; AV engelli version öğeleri yeniden denenmedi.

## Açık compiler CRT alanları

Recovered PTD initial-locale ile linked compiler CRT locale/errno/invalid-handler farklı alanlardır. NativeCalls hâlâ compiler locale codepage ve process ANSI/OEM seçimini okur; CrtApi raw function pointer’ları explicit locale/context owner taşımaz. B21 bunları **sabitlemez** ve hiçbir global locale, mbcp, File-API-mode veya handler setter’ı çağırmaz. Inherited OriginalEffects wide/CRT final yollarında başlangıç/leave kontrolü vardır; her iç çağrı için yeni bir checkpoint iddia edilmez.

B18/B19/B20 single-active PTD/admission, aynı Execution, shared stream, controller activation, stop/drain/join ve independent external quiescence şartları sürer. Yeni native gövde/kapanış **0**;487 mapped /491 eligible /124 normal claim /52 tarihsel kapanış korunur. Tam DLL hazırlığı **BLOCKED**; toplam DLL yüzdesi bu bileşimden türetilmez.

**B22:** global state değiştirmeden explicit owned compiler locale/invalid-copy provider ve sayısal codepage politikası. 7 online foreign çağrı, configuration/license/legacy document ve B10 tarihsel review borcu açık. Öncelik1→2→3→5→4; genel CRT kuyruğu bekler.

## B21 ortak doğrulama

**25 çekirdek + 27 entegrasyon = 52 kontrol PASS.** Ortak build 239 nesne (235 C++ / 4 C), 273 normal test grubu ve 19 framework terminal senaryosu içerir. Önceki 801 kaynak aynı; 4 önceki dosya açık hash kaydıyla güncellendi; toplam 810 kaynak. Yeni native gövde ve kapanış 0; 487 adres, 491 karşılaştırılabilir derlenmiş etiket ve 52 native kapanış korunur. Bileşim ile raw-all defterleri ayrıdır. Gerçek alt sağlayıcılar, foreign ABI, image unload ve PE girişi açık kalır.

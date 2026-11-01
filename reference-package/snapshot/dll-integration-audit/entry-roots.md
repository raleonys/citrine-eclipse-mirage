# B1 — DLL giriş/export/TLS kökleri

27 Ekim 2026. Bağımsız, yalnız kayıtlı kanıt incelemesi PASS. Yeni kaynak gövdesi/bridge/closure **0**; DLL linkleme veya runtime eşdeğerliği kanıtı değildir. Tam kimlik, hash ve kanıtlar [entry-roots.json](entry-roots.json) içindedir.

`anadius64.dll` özgün PE girişi **0x18004BCFC** (RVA4BCFC). Gerçek zincir:

`04BCFC → 04BBC8 → 038870 → 038970`

Girişte process-attach için cookie04C134 çağrısı vardır (`04BD18`); `04BD34` CRT dispatcher'a tail jump yapar. Dispatcher `04BC37` ile CRT reason yolunu, `04BC52` ile application DllMain038870'i çağırır. App gövdesi yalnız reason1'de038970'i çağırıp EAX1 döndürür (`038879/03887E`). Bu köklerin C++ normal algoritmaları entry.cpp/bootstrap.cpp içinde vardır; somut tek DLL sahibi ve tüm native bağları kurulmuş değildir.

## Export sözleşmesi

| Ordinal | Ad | Özgün RVA | Ortak gövde |
|---:|---|---|---|
|1|CDX|17A30|0x180017A30|
|2|RUNE|17A30|0x180017A30|
|3|anadius|17A30|0x180017A30|

Üç EAT girdisi aynı `RET 0` leaf gövdesini paylaşır; exportlardan biri app başlangıcı değildir. `ev_4ef5a2bb53b7728331fdd5c8debedafc4f7ebcce78b3dd4a30154920bfc23a9d` tam assembly kaydında bu leaf'in dönüş değeri ataması yoktur. Bir `.def` üç adı/ordinali tek yeniden kurulan sembole alias etmelidir. Bu denetim `.def` veya gövde üretmedi. Tercih edilmiş özgünVA bir host pointer'ına dönüştürülemez; isimlerden API imzası uydurulamaz.

Export dizini9F9D0/68 ve header entry, eski complete1024byte header ve complete138752byte rdata kayıtlarından bağımsız ayrıştırılıp recovery-index ile eşleştirildi. Parent SHA c6d1daab6e5cce17f48285d07f7b7a1f6c9be4b37f043f19694009393b401495; target SHA9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c.

## TLS ve başlangıç sahipliği

PE TLS dizini092080, template0959B8..0959C0 (8byte), index hücresi0A47E8, callback dizisi07F720'dir. İlk callback QWORD0; kayıtlı dizide loader TLS callback'i yoktur. Bu, TLS verisi olmadığı anlamına gelmez. CRT'nin ayrı dinamik callback slotu0A5A70'tir (`04C218 LEA`); hashed static-image-facts.json başlangıçta bunun zero-filled virtual data olduğunu kaydeder. İki slot karıştırılmamalıdır.

Native bootstrap `application_entry` SDK-tipli bir callback'tir; kendi `Binding` nesnesinin önceden kurulmuş olmasını ister ve girişte mutex/Lease kullanır. Header açıkça PE entry kurmadığını söyler. Dolayısıyla bunu doğrudan loader girişi yapmak somut global/kök kurulumunu çözmez. Outer `Entry` tüm module/reason/reserved argümanlarını korur; Bootstrap app projeksiyonu yalnız reason kullanır. Birleşik sahiplik ve loader bağlamı ayrıca kurulmalıdır.

## Gerçek link hedefinin kabul ölçütü

1. PE girişinden actual recovered CRT/application köklerine gerçek sembol zinciri ulaşmalı; export RET leaf'i veya kullanılmayan arşivler başarı göstergesi sayılmamalı.
2. Aynı ömre sahip cookie/onexit/PTD/CRT/application nesne grafiği ve zorunlu somut sağlayıcılar kurulmalı. Test mock'ları ve başarı döndüren boş gövdeler kullanılamaz.
3. Rebuilt initializer/terminator tabloları ve thread/TLS politikası tanımlanmalı; callback Binding kurulum sırası çözülmeli.
4. Son PE entry/üç export+ordinal/ortak alias EAT/TLS/import ve link-map statik denetlenmeli. Native loader-lock, EH/unwind ve worker kapanışı ayrı kapılardır.

Mevcut startup/ACRT/VCRT bridge'leri bazı eski pure yöntemleri kapatır; header'daki her pure bildirim global eksik sayılmaz. Ancak pair-bindings sınıfı `invoke_void_initializer`, `invoke_int_initializer`, `common_flush_all` sınırlarını açık bırakır. Birleşik nihai somut sağlayıcı denetimi ayrı B1 çalışma koludur.

## version.dll

Dış katman197assembly wrapper +1C++ loader ile198export/ordinal tarif eder; .def kaydı198girdidir. Dış app giriş VA005469; loader hâlâ **21.504 özgün gömülü baytı** include eder. Gömülü görüntü ayrı adres alanında entry003338 → CRT003210 → app/export001000 (`DllMain @1`) zincirine sahiptir; TLS dizini yoktur. Bağımsız kaynaklı embedded DLL kurulmadan dış katman source-only tamamlanmış değildir. Engelli original/generated version DLL'leri bu denetimde okunmadı veya yeniden denenmedi.

Orijinaller çalıştırılmadı/değiştirilmedi; mevcut CPP/HPP dosyaları korunur. Gerçek CFG/token/achievement içerikleri okunmadı. Yalnız yeni denetim JSON/Markdown ve üretici work betiği yazıldı.

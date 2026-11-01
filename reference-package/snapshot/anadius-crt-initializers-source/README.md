# CRT başlatıcı tabloları ve kaynak köprüsü

`initializers.cpp`, anadius64.dll SHA-256 `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c` için saklanan REA/Ghidra kanıtından dokuz gövdeyi yeniden yazar. Yeni `AcrtTablesBridge`, bunları dış CRT girişinin yedi zorunlu metoduna bağlar; ayrıca özgün bir fonksiyon kurtarılmış sayılmaz.

| Özgün VA | Davranış |
|---|---|
| `0x1800704f4` | Init/cleanup çiftlerini ileri başlatır; AL0 başarısızlıkta yalnız önceki başarılı/non-null init çiftlerinin cleanup'ını ters çağırır |
| `0x180070588` | Her non-null cleanup'ı ters çağırır; initializer'ın NULL olması bu yolda engel değildir |
| `0x180064614` | NULL atlayarak C++ void başlatıcı tablosunu ileri çağırır |
| `0x180064658` | C başlatıcı tablosunda EAX sıfır değilse o düşük32 bit sonucu döndürür |
| `0x180064594` | ACRT'nin çift tablosu başlatıcısı |
| `0x1800645cc` | Normal unload'da çiftlerin ters temizliği; termination'da yalnız qword `A4EF0!=0` ise flush(true) |
| `0x1800645a8` | Kurtarılmış optional PTD edinmesinin non-null sonucu |
| `0x1800645bc` | Kurtarılmış PTD serbest bırakma gövdesini çağırır |
| `0x180064604` | Kurtarılmış FLS/PTD uninitialize gövdesini çağırır |

Init çiftinde yalnız AL test edilir; C başlatıcısında EAX test edilir. Callback sırasında sonraki tablo girişleri değişebilir; her giriş ulaşıldığında okunur. Başarısız çiftin kendi cleanup'ı rollback'e katılmaz. Eksik/normal dönüş cleanup'ları exception durumuna genellenmez; MSVC SEH/unwind veya XFG protected-call ABI'si kurulmuş değildir. Callback exception'ı için ek rollback icat edilmez.

`initializer-tables.json`, SHA kontrolü sonrası özgün PE'den sınırlı salt okunur veriyi içerir: `0x18008aa60..0x18008ab60` içinde16 init/cleanup çifti, `0x18007f6d8..0x18007f710` içinde7 C giriş slotu, `0x18007f5d0..0x18007f6d0` içinde32 C++ giriş slotu. Raw offset, uzunluk ve byte hashleri kayıtlıdır. Bunlar **metadata kimlikleridir**; özgün VA'lar yürütülebilir pointer yapılmaz. Sembol eşleşmesi olmayan bir tablo girişi çözülmüş kaynak gövdesi sayılmaz. Bu yerel okuma yeni REA Evidence kaydı değildir.

Üretim API'sindeki span'ler yeniden derlenmiş callback bağlarını içerir. Bütün16 özgün init çiftinin alt sistem gövdeleri bu modül tarafından tamamlanmaz. VCRT, CPU, argv/environment, stdio/SList, C exit/validation ve uygulama davranışları zorunlu arayüz olarak kalır. Span storage'ı, PTD ve diğer state/provider nesneleri kullanım boyunca yaşamalı; eşzamanlı başlangıç/çıkış koordinasyonu caller'a aittir.

```powershell
& '.\outputs\anadius-crt-initializers-source\build.ps1'
```

904 kontrol: bütün256 AL değeri, başarısız konum/ters cleanup, NULL init/cleanup ayrımı, callback sırasında tablo değişimi, EAX yüksek bit farkı, qword flag'in yalnız yüksek32 bitinin set olması, termination/flush ve exception propagasyonu. Birleşik fixture gerçek kurtarılmış dış CRT→pair/C/C++ yürüyüşü→cookie/cache→uygulama kayıtları→onexit LIFO→PTD temizliği akışını kullanır. Onexit uygulama tablosu iki modülde aynı nesnedir. Init çiftleri bu fixture'da iki kontrollü kaynağa bağlanır; tüm16 özgün callback'in tamamlandığı veya gerçek uygulama/VCRT/OS backend'inin çalıştığı iddia edilmez.

Özgün/yeniden derlenen DLL veya oyun yüklenmez; OS/entropy/VCRT ve uygulama etkileri sentetiktir. Kanıtlar `evidence.json`, bounded assembly ve tablo JSON'u içindedir. `review.json`, bağımsız incelemede bulunan qword genişlik ve exception-test düzeltmelerinin çözümlerini kaydeder. Derleme/test sayısı tam DLL veya davranış eşdeğerliği değildir.

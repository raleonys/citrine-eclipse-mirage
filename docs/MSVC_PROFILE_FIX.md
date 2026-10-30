# MSVC profil loglarının düzeltmesi

Beş kullanıcı logu, önceki vendor doğrulama engelinin aşıldığını gösterir.
Release/Debug/Golden uygulama DLL'leri MSVC19.51 ile derlenip linklenmiştir.
Bu gözlem tam runtime, native test veya oyun kabulü değildir.

| Profil | Kullanıcı logundaki sonuç | Düzeltme / kalan iş |
|---|---|---|
| Compile | DLL ve runtime host derlendi, beş export doğru; resource allowlist üçüncü manifesti reddetti. | Ürün DLL'ine bütün MSVC profillerinde `/MANIFEST:NO` uygulanır. PNG ve VERSIONINFO denetimi aynı sıkılıkta kalır. |
| Debug | DLL ve runtime host derlendi; aynı ek manifest nedeniyle resource kontrolü durdu. | Aynı link kuralı Debug'a da uygulanır. Debug sembolleri korunur. |
| Native | Source API DLL/host dahil hedefler derlendi; execution testindeki `__int128` MSVC tarafından reddedildi. | Test oracle'ı dört32-bit çarpım ve signed high-half düzeltmesiyle standart64-bit aritmetiğe taşındı; uygulama aritmetiği değiştirilmedi. |
| CRT | Seçilmemiş execution testini de derleyen genel ALL hedefi aynı hatada durdu. | Oracle düzeltmesine ek olarak CRT yalnız kendi CRT/DLL test hedeflerini derler. Native profili tüm testleri derlemeye devam eder. |
| Golden | MSVC DLL/host derlemesi, export ve iki resource kontrolü başarılı; ürün hedefleri eksik, zincir başlamadan exit3. | Hedefler tamamlanmış sayılmaz. Operatör sonucu `BLOCKED` gösterir, exit3 ve özgün DLL'in değişmediği kontrolü korunur. |

MSVC'nin eklediği üçüncü resource `[24,2,1033]` RT_MANIFEST'tir. Kullanıcının
adopt ettiği iki resource `[10,101,1033]` SURGE PNG ve `[16,1,1033]` VERSIONINFO
olarak kalır. Resource dosyaları, vendor byte'ları ve beş export değişmez.
Source API geliştirme DLL'i de aynı manifest link kuralını alır; EXE manifestleri
bu ayardan etkilenmez. Metadata denetçisi manifesti kabul edecek şekilde gevşetilmez.

`LOCAL_BUILD_RECEIPT.json` artık build başarısı ile seçili profil doğrulamasını
ayrı kaydeder. Golden hedefleri eksikse doğrulama sonucu BLOCKED, exit3 olur;
bir derleme PASS'ı Golden PASS'ına çevrilmez.

Yeni kaynak için Windows'ta Compile/Debug/Native/CRT profilleri tekrar çalıştırılmalıdır.
Bu oturumdaki GCC, LLVM-MinGW/Wine ve kaynak QA kanıtı, düzeltilmiş kaynakların
gerçek MSVC yeniden çalıştırması olarak sunulmaz. Girdi loglarının hash'leri ve
güncel sonuçlar [MSVC_PROFILE_FIX.json](evidence/MSVC_PROFILE_FIX.json) içindedir.

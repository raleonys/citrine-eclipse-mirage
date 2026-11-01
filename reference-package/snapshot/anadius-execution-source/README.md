# Başlangıç adımlarının tamamlanması ve bekleme çekirdeği

Bu klasör, `anadius64.dll` içindeki üç uygulama yordamını ve kullandıkları dar mutex/zamanlama yollarını, adresli assembly kanıtından okunabilir C++20 olarak yeniden kurar. Bağımsız bir `anadius64.dll` değildir. Orijinal makine koduna delege etmez; yalnız işletim sistemi çağrıları ve henüz yeniden kurulmamış CRT hata yükseltme sınırı dışarıdan sağlanır.

İncelenen DLL SHA-256: `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`. Adresler bu DLL'nin `0x180000000` tercih edilen tabanına aittir. Ghidra 12.1.4/REA tarafından önceden alınmış `../anadius-all-functions.asm.txt`, `../anadius-all-functions.pseudo.c` ve `../anadius-recovery-index.json` kullanılmıştır. Bu turda hedef çalıştırılmadı, yüklenmedi veya değiştirilmedi.

| Kaynak karşılığı | DLL adresi | Assembly kanıtı / REA Evidence ID |
|---|---|---|
| `Execution::now_nanoseconds_036740` | `0x180036740` | `ev_be3ae7d8b4897a0ddc71c0b2cd532d595961166dddc6e3c474e298f45d37a109` |
| `Execution::finish_flag_036820` | `0x180036820` | `ev_629e77f34c0a3e467b712197fa2f04dd95b07ac77b702f01d06b287ca3c7a3a8` |
| `Execution::wait_steps_036880` | `0x180036880` | `ev_e6afa7f145235b765bf1eb49d88b97995634d7aa66bfe0e77febe5f38a52659b` |
| `initialize_mutex_04a698` | `0x18004a698` | `ev_d9a8ef2f70d04643eaeab013e98c16080c1fc75914b2de8e8d21731aefcda436` |
| `lock_04a6ac` giriş sarmalayıcısı | `0x18004a6ac` | `ev_b2ecd67949581475305c36cd518b31803cfdada8ed370caf518769c063b29972` |
| `lock_04a6ac` içindeki **null deadline** alt yolu | `0x18004a6d4` | `ev_3a286e7799c3c68d8967fbca7959d54c635796661e833479d15a8f0b7b30c921` |
| `unlock_04a6b4` | `0x18004a6b4` | `ev_f0e390709d3bf22112f7a6701b32a4244daee13ac34552aeee904c63586adc16` |
| Platform üzerinden QPC örneği | `0x18004a808` | `ev_26cbf80e2354269829804897e2ad25c4b9e6dc5f4740f89778fa715cf3bae963` |
| Frekans önbelleği ve Platform çağrısı | `0x18004a824` | `ev_5a9cd1ed62b74fc9001056c0a5e4827aa6047ac6e4b71eb3a3db6ad9b3c47b69` |
| `WindowsPlatform::sleep_milliseconds` sınırı | `0x18004a8bc` | `ev_9c9aaba1d4cc2062dd1ad0bc699917f1799a6df6e7fb11a222d2680ff3b4706a` |

## Korunan davranış

`finish_flag_036820`, `0x1800a3c40` mutex'ini alır; çağıranın verdiği bayrağa `1` yazar ve kilidi bırakır. Kilit hata döndürürse `0x18004a648` hata tablosu indeksi `5` istenir. Kilit sayacı `0x7fffffff` olduğunda sayaç `0x7ffffffe` yapılır ve indeks `6` istenir; bu hata yolunda bayrak yazılmaz veya kilit bırakılmaz. Hata yükselticisinin dönmemesi gerekir.

`wait_steps_036880`, kilit altında sırayla `0x1800a3c3d`, `0x1800a3c3c`, `0x1800a3c3e` bayraklarının sıfırdan farklı olmasını denetler. `count == -1`, üç adım anlamına gelir; diğer sıfır/negatif değerlerde denetim boş olduğu için sonuç doğru olur, fakat önce mutex yine alınır. Pozitif değerler ilk belirtilen adımları kapsar; 3'ten sonraki indisler bayrak değiştirmez. `INT_MAX` ve bütün bayrakların tamam olduğu uç durumda assembly'nin 32 bit sayaç sarılması sonsuz döngü oluşturur; kaynak bunu sessizce farklı bir sonuçla değiştirmez.

Tamamlanmamış durumda kilit bırakılır ve `0x18003697b` noktasındaki `20.000.000 ns` eklemesiyle son tarih hesaplanır. `0x180036982` eşiği `0x7ffffffffeced2ff` ve üzerindeki zamanları `INT64_MAX` değerine doyurur. İç döngü bayrakları okumaz: bir bayrağın Sleep sırasında tamamlanması mevcut 20 ms zaman aralığını kesmez. Yeni bayrak kontrolü iç zaman döngüsü tamamlanıp mutex yeniden alındığında yapılır.

QPC dönüşümü, 10 MHz için `counter * 100`; 24 MHz için magic-multiply bölme eşdeğeri; diğer frekanslar için bölüm/kalan hesabıdır. Kaynak düşük 64 bit çarpma/toplama/sarma davranışını açıkça korur. Uyku süresi kalan nanosaniyelerin milisaniyeye yukarı yuvarlanmasıdır; `86.400.000.000.000 ns` ve üzeri, tek seferde `86.400.000 ms` ile sınırlanır. Frekans önbelleği `0x1800a45e0` yalnız mevcut değeri sıfırken doldurulur.

`MutexState` gözlenen x64 MSVC düzenini tutar: SRW alanı `+0x10`, sahip iş parçacığı `+0x48`, sayaç `+0x4c`, toplam `0x50` bayt. Bu derleyicinin `std::mutex` ABI'si olarak kullanılmaz. `04a698` yalnız gözlenen alanları yazar; belirsiz baytlar korunur. `04a6ac` kesin olarak `EDX=0` ile `04a6d4`'e gider. Kaynak yalnız bu sınırsız bekleme yolunu uygular; genel timed/try-lock yollarının kurtarıldığı iddia edilmez.

## Derleme ve doğrulama

Çalışma alanı kökünden:

```powershell
& .\outputs\anadius-execution-source\build.ps1
```

Gerekirse `-ToolchainRoot` ile LLVM-MinGW klasörü verilir. Script C++20, `-O2 -Wall -Wextra -Werror` ile `execution.o`, `windows_platform.o` ve bağımsız sentetik test EXE'sini derler. Yalnız kendi mock test EXE'sini çalıştırır. Windows adaptörü derlenir, testlere bağlanmaz ve bu turda çalıştırılmaz. Derleyici kimliği, kaynak/kanıt/destek nesnesi hashleri ve gerçek kontrol sayısı `validation.json` dosyasına yazılır.

Testler mutex alan düzeni ve korunmuş baytları, recursion/failure/overflow yollarını, sekiz bayrak bileşiminin prefix davranışını, QPC frekans önbelleğini, uyku yuvarlama/doyma sınırlarını ve kilidin bırakıldığı sırada bayrak değişimini kapsar. 24 MHz hesabı ayrıca uç değerlerde ve 10.000 deterministik girdide, assembly'deki yüksek çarpım/SAR/SHR adımlarından ayrı yazılmış aritmetik oracle ile karşılaştırılır. Bunlar **sentetik sözleşme ve aritmetik kontrolleridir**; orijinal DLL ile runtime karşılaştırması veya tüm DLL'nin eşdeğerlik kanıtı değildir.

## Kalan sınırlar

Global bayrakların, mutex türünün/içeriğinin ve frekans önbelleğinin başlangıç değerlerini çağıran sağlamalıdır. Kurucu bunları tahmin ederek sıfırlamaz. Gerçek başlangıç zinciri, DLL ABI/export/CRT entegrasyonu ve kalan uygulama modülleri burada birleştirilmemiştir.

`Platform` test edilebilir bir yeniden kurma arayüzüdür; orijinal sınıf arayüzü değildir. `WindowsPlatform` QPC, Sleep ve SRW API'lerine bağlanır; `0x18004a648` CRT exception üretiminin ABI uyumlu karşılığı çağıranın zorunlu hata yükselticisidir. Sahiplik/sayaç alanlarında orijinal makine kodu gibi atomik olmayan erişimler vardır; C++ bellek modeli altında genel çok iş parçacıklı eşdeğerlik gösterilmemiştir. Geçerli pozitif QPC frekansı beklenir: başarısız QPF/QPC çağrıları ve sıfıra/`INT64_MIN / -1` bölme CPU trap davranışını C++ exception olarak yeniden üretme bu modülün kapsamı dışındadır.

Özgün isimler, kaynak dosya düzeni ve derleyici çıktısının ikili eşitliği kurtarılmış değildir. Mac sembolleri kavramsal isimleri destekleyebilir; bu dosyalar için Windows davranışının dayanağı yukarıdaki adresli Windows assembly kanıtıdır.

# CRT PTD / FLS yaşam süresi

`anadius64.dll` SHA-256 `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c` için saklanan REA/Ghidra pseudocode ve assembly kanıtlarından yeniden yazıldı. Orijinal kaynak veya gerçek CRT/FLS ABI eşdeğerliği iddiası değildir.

`ptd.hpp/cpp` yedi işlevin kontrol akışını kapsar:

| Adres | Yeniden yazılan işlev |
|---|---|
| `0x180064b4c` | Geçerli FLS indeksinde değeri al; sıfırla; PTD içeriğini yok et; belleği serbest bırak |
| `0x180064b90` | PTD al/oluştur, normal akışta LastError koru; başarısızlıkta zorunlu abort sağlayıcısı |
| `0x180064c64` | LastError korumadan zorunlu PTD al/oluştur |
| `0x180064d08` | LastError koruyan isteğe bağlı PTD al/oluştur |
| `0x180064dd0` | LastError korumayan isteğe bağlı PTD, yalnız durum indeksi 0 |
| `0x180064e90` | FLS callback kaydı, ilk PTD oluşturma ve başarısızlık temizliği |
| `0x180064ecc` | FLS indeksini bırak, sonucu yok say ve indeksi `0xffffffff` yap |

PTD 0x3c8 bayttır; `thread_context` alanı +0x3c0 konumunda gerçek tipli `threads_recovered::Context*` nesnesidir. Diğer alanlar bu katmanda bayt deposudur. Sağlayıcı sıfırlanmış ve yaşam süresi başlamış bir `Ptd` veya null döndürmelidir. `calloc`/heap uyumluluk katmanı zorunlu dış bağımlılık olarak kalır.

Yapım sırasında FLS'ye `-1` işaretçisi yazılır. Yeniden giriş bunu null sonucuna çevirir. Başarılı tahsis kurucudan önce yayımlanır. Başarısız `FlsSetValue(..., nullptr)` çağrıları yok sayılır; tahsis başarısızlığında ve başarılı yapımdan sonra `free(nullptr)` çağrıları korunur. Geçersiz FLS indeksi alma çağrısını atlar fakat sentinel yazmayı yine dener. `free_ptd` sentinel için ayrı koruma eklemez; bu durum yalnız opak sağlayıcılarla sınanır ve gerçek yıkıcı için güvenli kabul edilmez.

LastError yalnız `064b90` ve `064d08` normal dönüşlerinde geri yüklenir; `064b90` abort öncesinde de geri yükler. İstisna halinde yeni RAII geri alma, PTD sıfırlama veya LastError geri yükleme eklenmez. `064e90` ve `064ecc` için özgün assembly yalnız AL boolean sonucunu tanımlar; yüksek dönüş bitleri modellenmez. `064dd0`'ın diğer durum indeksleri orijinalde `base + index*0x3c8` hesabına gider; tek nesne modeli bu indekslerin geçerli depolamasını kanıtlamadığı için genel indeks API'si sunulmaz.

FLS kaydı `FlsCallback::destroy_0649cc` tanımlayıcısı kullanır; özgün tercih edilen adres çalıştırılmaz. FLS API, tahsis, kurucu/yıkıcı ve abort işlemlerinin tamamı zorunlu `Effects` sözleşmesidir; başarı taklidi yapan üretim sağlayıcısı yoktur.

`build.ps1` Clang ile üretim nesnesini derler ve yalnız kendi sentetik test yürütülebilirini çalıştırır. **1510 kontrol geçti.** Dört PTD edinim yolunun hata matrisi, sentinel yeniden giriş, yayımlama sırası, korunmayan istisna yolları, başarısız temizleme, yıkıcı/silme sırası, FLS başlangıç/çıkış ve +0x3c0 alan yerleşimi kapsanır. Gerçek FLS, DLL, oyun veya modül sağlayıcısı çalıştırılmaz.

Kanıt kimlikleri ve kaynak özetleri `evidence.json` içindedir; yedi işlevin saklanan assembly blokları `assembly-evidence.txt` içinde bulunur. Gerçek Windows FLS/heap bağdaştırıcısı ve diğer CRT alanları ayrı çalışma gerektirir.

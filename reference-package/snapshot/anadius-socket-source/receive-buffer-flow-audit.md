# Receive tamponu — 0x180010040 / 0x180008350

R1 aşaması **iki yeni normal gövde** ekler: `resize_receive_buffer_010040` ve `grow_receive_buffer_008350`. `receive_buffer.hpp/cpp`, mevcut `WriteFinishString` tanımlı alan görünümünü paylaşır; ayrı ve birbiriyle tutarsız ikinci bir string modeli oluşturulmaz. Native MSVC STL/allocator/ABI bağlantısı henüz tamamlanmadı.

Hedef SHA-256: `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`. REA 6.5.0 sunucu kimliği ve Ghidra 12.1.4 kullanılabilirliği doğrulandı; aynı hedefe bağlı saklanan kanıtlar yeniden kullanıldı. Yeni native analiz başlatılmadı. Oturum kaydı `receive-buffer-rea-session.json` içindedir.

| Gövde | Pseudocode kanıtı | Assembly kanıtı |
|---|---|---|
| `010040` | `ev_8b172ce1ae7a2a8f6e7d19d9433d0f9681d58a8e0fba88dbe42da06deb17bac4` | `ev_4eed657fe242e320b79de3469b4ff268d87d2033487b16f14133e304afc75ba1` |
| `008350` | `ev_8f414a470d18574ce283ec7f9d33a326dcabef0214a8d5a54fdfd5f5b9bba010` | `ev_36491f90ef8a29d2fb8edd6b1735250524629ef5f6e005c31809a72ed253a8f3` |

## Boyutlandırma ve büyüme sözleşmesi

Adreslerde ortak `0x180` öneki kısaltılmıştır. Tam kesitler `receive-buffer-evidence.*`, kullanılan eski yardımcılar `receive-buffer-context.*` dosyalarında tutulur.

| Adres | Gözlenen davranış ve kaynak karşılığı |
|---|---|
| `010044..01005b` | Eski boyut saklanır. İstenen boyut küçük/eşitse size yazılır; capacity>15 ise heap pointer seçilir, yeni sonuna NUL yazılır. Kapasite küçültülmez ve allocation/free yapılmaz. |
| `010064..0100a3` | `extra = target-old_size` ve `capacity-old_size` unsigned64 karşılaştırılır. Kapasite içinde büyümede önce size yazılır, eski sona fill yapılır, saklanan tail+extra konumuna NUL yazılır. |
| `0100b6..0100c4` | Eksik kapasitede gerçek `008350`; extra ve fill_count aynı, kullanılmayan üçüncü argüman0 olarak aktarılır. |
| `00835b..008378` | Eski size saklanır; `MAX_INT64-old_size < extra` ise `00ce80` length-error sınırı. Tahsisten önce gelir. |
| `008396..0083c1` | Yeni kapasite, `(new_size OR 15)` ile `old_capacity + floor(old_capacity/2)` değerlerinin büyüğüdür. Geometrik sonuç ayrıca16 sınırına yuvarlanmaz. Limit koşullarında kapasite MAX_INT64'e doyar. |
| `0083c5..008404` | `capacity+1 <4096` için doğrudan tahsis. Büyük tahsiste `capacity+1+39` istenir; null sonuç `05c974` sınırına gider. Data `(raw+39)&~31`, data-8 header ise raw base olur. Doygun istek `0x8000000000000027` bayttır. |
| `0083d1..0083d8` | Büyük allocation size taşma kontrolü korunur. Geçerli MAX_INT64 kapasite alanında bu dalın tetiklenmesi beklenmez; testlerde ulaşılmış gibi sunulmaz. |
| `008427..00843f` | Yeni size ve capacity, copy/fill öncesinde yazılır. Eski heap pointer tahsisten ve bu yazımlardan sonra canlı prefix'ten alınır; eski size/capacity ise girişte saklanan değerlerdir. |
| `008442..00849f` | Eski size kadar byte copy, bağımsız fill_count kadar fill ve onun sonuna NUL. Doğrudan büyüme API'sinde fill_count ile extra zorla eşitlenmez. |
| `008455..008482` | Eski kapasitenin `+1` boyutu kullanılır; büyük eski tahsiste `+39` ve header base. Unsigned `old_data-base-8 <=31` doğrulanır: data-base aralığı8..39. Assembly çağrı hedefi `04b0f8` ve boyut argümanı korunur; pseudocode'un tek argümanlı thunk görünümü boyut bilgisini kaybettirmez. |
| `0084a4` | Yeni pointer yalnız copy/fill/NUL ve eski heap free tamamlandıktan sonra prefix'e yazılır. Hata prefix'lerine otomatik rollback eklenmez. |

`ReceiveBufferMemory` canlı inline prefix, heap ve header bölgelerini salt bellek eşlemesi olarak kullanır. Eksik/çift eşleme `UnsupportedInput` üretir; bu C++ hataları native fault veya exception ABI'si değildir. Bütün nesneler son native kullanıma kadar canlı kalmalıdır.

`BoundedReceiveBufferApi`, `078d30` copy ve `0793d0` fill çağrılarının gereken byte sözleşmesini sahipli span'larda uygular. **Bu iki genel CRT/SIMD gövdesi kurtarıldı sayılmaz.** Yöntemler bu nedenle yeni native fonksiyon adıyla etiketlenmez. Allocator, sized-free, length/allocation-overflow/invalid-parameter terminal sağlayıcıları zorunlu kalır. Genel CRT üretim kaynakları değiştirilmedi.

## Doğrulama

**3.977 çekirdek + 684 entegrasyon = 4.661 yeni kontrol PASS.** Bağımsız `std::string` byte oracle'ı yalnız içerik ve boyutu karşılaştırır; kapasite için açık native anchor değerleri kullanılır. Inline/heap, küçültme, kapasite içi büyüme, 4095/4096 allocation sınırı, 512/65536 receive istekleri, NUL ve 0xff fill, header gap, tahsis hatası ve güncellenmiş metadata/eski pointer hata prefix'leri kapsanır.

MAX_INT64/doyma sınırları seyrek bellek pencereleriyle yalnız integer ve kontrol akışı açısından sınanır; devasa bellek tahsisi/copy yapıldığı iddia edilmez. Küçük/büyük null tahsis yolları ayrı tutulur. Sahipli fixture'larda eski allocation byte depolarının ömrü gerçekten sonlandırılır.

Entegrasyon gerçek `010040/008350 → 021d10 → 023dd0 → enjekte SDK → 027710 → 021a40` kaynaklarını birleştirir. Gelen byte, embedded NUL, EOF ve mapped error ile sahiplik temizliği sınanır. Prepare/commit çağırıcısı kontrollü test harness'idir; **`025c40` gövdesi henüz kurtarılmış değildir**.

Güncel doğrulama yolu ortak `source-workspace/build.ps1` kaydıdır. Aynı C++ gövdelerini ikinci kez sınayan tam standalone socket script'i bu aşamada yeniden çalıştırılmaz. Önceki21 yerel doğrulama kaydı önceki config hashleriyle tarihsel olarak korunur; yeni stage kaydı ortak build sonuçlarına bağlanır. Ortak build bütün44 socket test grubunu kapsar.

Kaynak/assembly karşılaştırması ve testleri aynı ajan hazırladı; ayrı bağımsız reviewer onayı yoktur. Tam native STL/heap/EH/SEH/concurrency/ABI veya DLL eşdeğerliği kanıtlanmadı. Özgün DLL/oyun çalıştırılmadı veya değiştirilmedi; gerçek CFG/token/achievement dosyaları okunmadı.

Toplam DLL için doğrulanmış yüzde yoktur. Bu aşamanın somut artışı iki normal gövdedir. Sıradaki R3 işi **0x180025c40 composed receive/read-until** ve R1/R2 kaynak bağlantısıdır; ardından native frame tamamlaması gelir.

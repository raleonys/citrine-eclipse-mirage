# Native locale referansları ve derin kaynak temizliği

Yeni `Resources`, mevcut `crt_locale_resource_bridge::Resources` sınıfına kurtarılmış ACRT lock/unlock girişlerini ve kayıtlı gerçek `LONG`/Interlocked sağlayıcısını bağlar. Önceki native sync testindeki basit locale ref sağlayıcısının yerine gerçek `070634/0708c0` ve locale/time/lconv/ctype/category free kaynakları çalışır. Yeni çıkarılmış özgün gövde sayısı **0**; bu çalışma mevcut kaynakların native bağlantısıdır.

Memory, globals, identities/defaults, heap, kilit bankaları ve counter registry callback'ler tamamlanana kadar yaşamalıdır. Counter registry'ye yalnız yaşayan typed `LONG&` bağlanır. Registry kullanıcılarından herhangi biri çalışırken bind/unbind yapılmaz; yalnız serbest bırakılan counter'ın son referans olması yeterli değildir. Global yayımlama ayrıca caller tarafından kilitlenir.

## Doğrulama

`build.ps1` LLVM-MinGW/C++20/O2/Wall/Wextra/Werror ile kendi EXE'sini derler: **750 kontrol PASS**. Bağımsız inceleme `review-native-locale.json` dosyasındadır; ortak proje ayrı `common-validation.json` üretir. CMake ayrıca çalıştırılmış sayılmaz.

- Typed fixture kayıtları tüketilen offsetleri kapsar: locale0x160/ref+0x10, time0x2c0/ref+0x15c ve kategori0x20. Bunlar `static_assert` ile kontrol edilir; tam özgün C++ tiplerinin veya PE başlangıç verilerinin kurtarıldığı anlamına gelmez.
- Dinamik locale ağacı **133 ayrı heap bloğu ve 18 counter binding** içerir: lconv ve18 string yaprağı, dört ayrı ref bloğu, üç ctype/case taban tamponu, time ve87 yaprağı, altı kategori metni ve12 kategori ref bloğu. Kategori adı ile extra-text aynı metin bloğunu paylaşır; diğer sahipli bloklar ayrıdır.
- Dinamik sayaçlar fixture global sahibini temsil eden1 ile başlar. Main PTD kurulunca2, dört worker aynı anda canlıyken6, worker callback'leri sonrası2 olur. MB sayacı aynı anda5'tir. Peak'te133 ağaç bloğu ve5PTD, toplam138 canlı blok vardır. Her worker'ın errno/LastError alanı ayrı doğrulanır.
- Worker'lar join edildikten sonra gerçek `0709d8` global locale pointer'ını ve ref'leri değiştirir: eski ağaç1, yeni initial locale1. Main flag'i refresh yoluna izin verecek şekilde ayarlanır; gerçek `070968/0709d8` eski ağacı0'a indirip **133 bloğu** derin free gövdeleriyle bırakır. Yeni initial locale2 olur. Main FLS callback sonrası1, fixture global sahibinin kaynak release'i sonrası0 olur.
- Typed LONG kayıtları gerçek HeapFree'den **önce**, nesne hâlâ canlıyken kaldırılır. Free başarısız olursa gözlem sarmalayıcısı canlı kayıtları geri bağlar; bu stage'de failed-free enjekte edilmez. Worker'lar yalnız counter içermeyen PTD bloklarını bırakır; registry map'i worker süresince değişmez. Dinamik ağacın unbind/free işlemleri yalnız join sonrası yapılır.
- Aynı test modern lock API ve export yokluğu enjekte edilmiş fallback yolunda çalışır. Gerçek FLS, process heap, kritik bölümler ve Interlocked kullanılır; loader yalnız sabit Windows sistem modüllerini yükler.

Memory alanları her kaynak okumasında taze okunur; locale/lconv/time kaydı topluca cache'lenmez. Gözlem map'i mutex ile korunur; callback'lerde PTD free ile başka worker'ın locale okuması çakışabilir. Ayrı counter kayıtları bu sırada mutasyona uğramaz. Quiescent tree disposal sonunda bütün18 dinamik binding kaldırılmış ve yalnız main PTD kalmış olduğu kontrol edilir.

Fixture initial/default/global kimlikleri kontrollüdür. Özgün initial locale/global görüntüsü, constructor/loader sırası veya tüm alanların gerçek başlangıç değerleri bu aşamada çıkarılmış değildir. Arbitrary alias/double-free, eşzamanlı global değiştirme, worker çalışırken tree disposal, tam locale kurucuları, genel CRT concurrency, MSVC ABI ve SEH/unwind eşdeğerliği açık kalır. Orijinal veya yeniden derlenen DLL/oyun çalıştırılmaz; özgün girdiler değiştirilmez.

## Kanıt bağlantısı

`0x180070634/0708c0/070838/070898` referans; `0x1800706c0/070a40/071068/070860/071730/0713bc` derin free; `0x1800709d8/070968` global/thread değiştirme; `0x180064ae4/0648fc/0649ec/0649cc` PTD bağlantısı. Hash bağlı statik dayanaklar `evidence.json` içinde mevcut REA kayıtlarına bağlanır. Bu adresler native callback pointer'ları olarak çalıştırılmaz.
